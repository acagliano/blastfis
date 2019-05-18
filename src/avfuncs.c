
#include <string.h>
#include <debug.h>
#include <fileioc.h>
#include <graphx.h>
#include "indexing.h"
#include "crypto.h"
#include "avfuncs.h"

const char *PropDB = "AVPropDB";
const char *AvDB = "AVDefsDB";
const char *SnapDB = "AVSnapDB";


int progsort(const void* a, const void* b){
    char i;
    char a_letter, b_letter;
    char diff;
    progname_t* pa = (progname_t*)a;
    progname_t* pb = (progname_t*)b;
    for(i = 0; i < 8; i++){
        a_letter = pa->name[i];
        b_letter = pb->name[i];
        diff = a_letter - b_letter;
        if ((diff != 0) && (diff != 32) && (diff != -32)) return diff;
    }
    return 0;
}


int av_GetNumFiles(void){
    char* var_name;
    int count = 0;
    uint8_t *search_pos = NULL;
    uint8_t type;
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL){
        switch(type){
            case TI_PRGM_TYPE:
            case TI_PPRGM_TYPE:
            case TI_APPVAR_TYPE:
                if(strncmp(var_name, "#", 8) && strncmp(var_name, "!", 8) && strncmp(var_name, PropDB, 8)) count++;
                break;
        }
    }
    return count;
}

int av_GenerateFileIndex(progname_t* prognames, int count){
    int i = 0;
    char* var_name;
    uint8_t *search_pos = NULL;
    uint8_t type;
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL){
        unsigned int complete = (100 * i / count);
        gfx_SetColor(255);
        gfx_FillRectangle(0, 0, 320, 25);
        gfx_PrintStringXY("Indexing device contents...", 5, 5);
        gfx_SetColor(0);
        gfx_FillRectangle(0, 20, 320 * i / count, 5);
        gfx_SetTextXY(195, 5);
        gfx_PrintUInt(complete, 1 + (complete > 9) + (complete > 99));
        gfx_PrintString("%");
        gfx_BlitBuffer();
        switch(type){
            case TI_PRGM_TYPE:
            case TI_PPRGM_TYPE:
            case TI_APPVAR_TYPE:
                if(strncmp(var_name, "#", 8) && strncmp(var_name, "!", 8) && strncmp(var_name, PropDB, 8)){
                    ti_var_t openfile;
                    progname_t* prog = &prognames[i++];
                    prog->type = type;
                    memcpy(prog->name, var_name, 8);
                    if(openfile = ti_OpenVar(prog->name, "r", type)){
                        int value = 0; unsigned long checksum = 0;
                        prog->prop_track = (unsigned int)av_LocateFileInPropDB(prog);
                        prog->size = ti_GetSize(openfile);
                        prog->checksum = rc_crc32(0, ti_GetDataPtr(openfile), prog->size);
                        ti_Close(openfile);
                        if(!strncmp("BLASTFIS", var_name, 8))
                            if(!av_LocateFileInSnapDB(prog))
                                av_CreateSnapshot(prog);
                    }
                }
        }
    }
    qsort(prognames, count, sizeof(progname_t), progsort);
    return 1;
}

progsave_t* av_LocateFileInPropDB(progname_t* program){
    size_t db_size;
    int i;
    progsave_t* db_data = av_FileGetPtr(PropDB, TI_APPVAR_TYPE, &db_size);
    db_size /= sizeof(progsave_t);
    for(i = 0; i < db_size; i++){
        progsave_t* this = &db_data[i];
        if((!strncmp(this->name, program->name, 8)) && (this->type == program->type))
            return this;
    }
    return NULL;
}


void* av_FileGetPtr(const char *name, uint8_t type, size_t *Len){
    ti_var_t fp = ti_OpenVar(name, "r+", type);
    void* addr;
    if(!fp) return NULL;
    *Len = ti_GetSize(fp);
    addr = ti_GetDataPtr(fp);
    ti_Close(fp);
    return addr;
}

size_t av_ResizeFile(const char *name, uint8_t type, int sizealt){
    ti_var_t fp;
    size_t fp_size = 0;
    if(fp = ti_OpenVar(name, "r+", type)){
        fp_size = ti_Resize(ti_GetSize(fp) + sizealt, fp);
        ti_Close(fp);
    }
    return fp_size;
}


void av_TogglePropTrack(progname_t* program){
    if(program->prop_track == 0) enable_PropTrack(program);
    else disable_PropTrack(program);
}

void enable_PropTrack(progname_t* program){
    ti_var_t dbfile;
    int i;
    progsave_t tmp;
    size_t size;
    progsave_t* db_data = av_FileGetPtr(PropDB, TI_APPVAR_TYPE, &size);
    strncpy(tmp.name, program->name, 8);
    tmp.type = program->type;
    tmp.size = program->size;
    tmp.checksum = program->checksum;
    size /= sizeof(progsave_t);
    for(i = 0; i < size; i++){
        progsave_t* this = &db_data[i];
        if(!this->type) break;
    }
    if(!(dbfile = ti_Open(PropDB, "r+"))) return;
    ti_Seek(i * sizeof(progsave_t), SEEK_SET, dbfile);
    ti_Write(&tmp, sizeof(progsave_t), 1, dbfile);
    ti_Close(dbfile);
    program->prop_track = (unsigned int)av_LocateFileInPropDB(program);
}

void disable_PropTrack(progname_t* program){
    progsave_t* dest;
    if(dest = av_LocateFileInPropDB(program)){
        memset(dest, 0, sizeof(progsave_t));
        program->prop_track = 0;
    }
}

void av_UpdateAttributes(progname_t* program){
    progsave_t* save = av_LocateFileInPropDB(program);
    save->checksum = program->checksum;
    save->size = program->size;
}

void av_ToggleSnapshot(progname_t* program){
    if(av_LocateFileInSnapDB(program)) av_DeleteSnapshot(program);
    else av_CreateSnapshot(program);
}

void av_DeleteSnapshot(progname_t* program){
    size_t size;
    unsigned int start = (unsigned int)av_FileGetPtr(SnapDB, TI_APPVAR_TYPE, &size);
    unsigned int end = start + size;
    snapshot_t* dest = (snapshot_t*)av_LocateFileInSnapDB(program);
    size_t itemsize = dest->size;
    snapshot_t* src = (snapshot_t*)((size_t)dest + itemsize);
    size_t copybytes = end - (size_t)src;
    if(!strncmp("BLASTFIS", program->name, 8)) return;
    dbg_sprintf(dbgout, "Start: %u\n", start);
    dbg_sprintf(dbgout, "End: %u\n", end);
    dbg_sprintf(dbgout, "File Size: %u\n", size);
    dbg_sprintf(dbgout, "Dest: %u\n", dest);
    dbg_sprintf(dbgout, "Src: %u\n", src);
    dbg_sprintf(dbgout, "item size: %u\n", itemsize);
    dbg_sprintf(dbgout, "Bytes to copy: %u\n", copybytes);
    if(dest == 0) return;
    if(copybytes) memcpy(dest, src, copybytes);
    if(!av_ResizeFile(SnapDB, TI_APPVAR_TYPE, (-itemsize))) dbg_sprintf(dbgout, "Resize Failed!\n");
}

void av_CreateSnapshot(progname_t* program){
    ti_var_t dbfile;
    snapshot_t tmp;
    size_t size;
    progsave_t* db_data = av_FileGetPtr(SnapDB, TI_APPVAR_TYPE, &size);
    if(db_data ==NULL) return;
    strncpy(tmp.fname, program->name, 8);
    tmp.type = program->type;
    tmp.size = program->size + sizeof(snapshot_t);
    boot_GetDate(&tmp.time.day, &tmp.time.month, &tmp.time.year);
    if(dbfile = ti_Open(SnapDB, "r+")){
        ti_var_t srcfile;
        ti_Seek(0, SEEK_END, dbfile);
        ti_Write(&tmp, sizeof(snapshot_t), 1, dbfile);
        if(srcfile = ti_OpenVar(program->name, "r", program->type)){
            ti_Write(ti_GetDataPtr(srcfile), ti_GetSize(srcfile), 1, dbfile);
            ti_Close(srcfile);
        }
        ti_Close(dbfile);
    }
}

unsigned int av_LocateFileInSnapDB(progname_t* program){
    size_t size;
    unsigned int db_data = (unsigned int)av_FileGetPtr(SnapDB, TI_APPVAR_TYPE, &size);
    unsigned int db_end = db_data + size;
    if(db_data == NULL) return 0;
    while(db_data < db_end){
        snapshot_t* this = (snapshot_t*)db_data;
        if((!strncmp(this->fname, program->name, 8)) && (this->type == program->type))
            return (unsigned int)this;
        db_data += this->size;
    }
    return 0;
}

void av_UpdateSnapshot(progname_t* program){
    
}
