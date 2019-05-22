
#include <string.h>
//#include <debug.h>
#include <fileioc.h>
#include <graphx.h>
#include "indexing.h"
#include "crypto.h"
#include "avfuncs.h"

const char *PropDB = "AVPropDB";
const char *AvDB = "AVDefsDB";
const char *SnapDB = "AVSnapDB";
const char *SnapFile = "AVshXXXX";
const char *AVSettings = "AVSett";


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


uint24_t av_GetNumFiles(void){
    char* var_name;
    uint24_t count = 0;
    uint8_t *search_pos = NULL;
    uint8_t type;
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL){
        switch(type){
            case TI_PRGM_TYPE:
            case TI_PPRGM_TYPE:
            case TI_APPVAR_TYPE:
                if(strncmp(var_name, "#", 8) &&
                   strncmp(var_name, "!", 8) &&
                   strncmp(var_name, PropDB, 8) &&
                   strncmp(var_name, SnapDB, 8) &&
                   strncmp(var_name, "AVsh", 4)) count++;
                break;
        }
    }
    return count;
}

int av_GenerateFileIndex(progname_t* prognames, uint24_t count){
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
                if(strncmp(var_name, "#", 8) &&
                   strncmp(var_name, "!", 8) &&
                   strncmp(var_name, PropDB, 8) &&
                   strncmp(var_name, SnapDB, 8) &&
                   strncmp(var_name, "AVsh", 4)){
                    size_t size;
                    settings_t* s = (settings_t*)av_FileGetPtr(AVSettings, TI_APPVAR_TYPE, &size);
                    ti_var_t openfile;
                    progname_t* prog = &prognames[i++];
                    prog->type = type;
                    memcpy(prog->name, var_name, 8);
                    if(s->indexSplit) break;
                    av_TellAttributes(prog);
                }
        }
    }
    qsort(prognames, count, sizeof(progname_t), progsort);
    return 1;
}

void av_TellAttributes(progname_t* program){
    ti_var_t openfile;
    if(openfile = ti_OpenVar(program->name, "r", program->type)){
        int value = 0; unsigned long checksum = 0;
        program->prop_track = (unsigned int)av_LocateFileInPropDB(program);
        program->size = ti_GetSize(openfile);
        program->checksum = rc_crc32(0, ti_GetDataPtr(openfile), program->size);
        ti_Close(openfile);
        program->indexed = true;
    }
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

void* av_FileGetEnd(const char *name, uint8_t type){
    ti_var_t fp = ti_OpenVar(name, "r+", type);
    void* addr;
    if(!fp) return NULL;
    ti_Seek(0, SEEK_END, fp);
    addr = ti_GetDataPtr(fp);
    ti_Close(fp);
    return addr;
}

size_t av_ShrinkFile(const char *name, uint8_t type, unsigned int sizealt){
    ti_var_t fp;
    size_t fp_size = 0;
    if(fp = ti_OpenVar(name, "r+", type)){
        unsigned int newsize = ti_GetSize(fp) - sizealt;
        fp_size = ti_Resize(newsize, fp);
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


uint16_t av_DeleteSnapshot(uint16_t index, uint16_t count){
    ti_var_t dbfile, snapfile;
    char snapname[9];
    uint16_t slots_to_copy = count - (index + 1);
    int i;
    if(index != -1){
        strncpy(snapname, SnapFile, 8);
        sprintf(snapname + 4, "%04x", index);
        if(ti_Delete(snapname)){
            if(dbfile = ti_Open(SnapDB, "r+")){
                snapshot_t *dest, *src;
                ti_Seek(index * sizeof(snapshot_t), SEEK_SET, dbfile);
                dest = (snapshot_t*)ti_GetDataPtr(dbfile);
                memset(dest, 0, sizeof(snapshot_t));
                src = dest + 1;
                for(i = 0; i < slots_to_copy; i++)
                    memcpy(dest++, src++, sizeof(snapshot_t));
                ti_Close(dbfile);
                count--;
            }
        }
    }
    return count;
}

void av_CreateSnapshot(progname_t* program){
    char snapname[9];
    ti_var_t snapfile, srcfile;
    uint16_t index;
    snapshot_t tmp = {0};
    size_t size;
    strncpy(tmp.fname, program->name, 8);
    tmp.type = program->type;
    tmp.size = program->size;
    boot_GetDate(&tmp.time.day, &tmp.time.month, &tmp.time.year);
    index = av_SnapDB_GetOpenSlotOffset();
    strncpy(snapname, SnapFile, 8);
    sprintf(snapname + 4, "%04x", index);
    if(snapfile = ti_Open(snapname, "w")){
        if(srcfile = ti_OpenVar(program->name, "r", program->type)){
            if(ti_Write(ti_GetDataPtr(srcfile), ti_GetSize(srcfile), 1, snapfile)){
                ti_var_t dbfile;
                if(dbfile = ti_Open(SnapDB, "r+")){
                    ti_Seek(index * sizeof(snapshot_t), SEEK_SET, dbfile);
                    ti_Write(&tmp, sizeof(snapshot_t), 1, dbfile);
                    ti_Close(dbfile);
                }
            }
            ti_Close(srcfile);
        }
        ti_Close(snapfile);
    }
}

void av_RestoreSnapshot(uint16_t index){
    char snapname[9];
    ti_var_t destfile, snapfile;
    snapshot_t *ref = av_SnapDB_IndexToPtr(index);
    strncpy(snapname, SnapFile, 8);
    sprintf(snapname + 4, "%04x", index);
    if(destfile = ti_OpenVar(ref->fname, "w", ref->type)){
        if(snapfile = ti_Open(snapname, "r")){
            ti_Write(ti_GetDataPtr(snapfile), ti_GetSize(snapfile), 1, destfile);
            ti_Close(snapfile);
        }
        ti_Close(destfile);
    }
}

void av_UpdateSnapshot(progname_t* program, uint16_t index){
    size_t size;
    snapshot_t* dbstart = av_FileGetPtr(SnapDB, TI_APPVAR_TYPE, &size);
    size /= sizeof(snapshot_t);
    av_DeleteSnapshot(index, size);
    av_CreateSnapshot(program);
}


int16_t av_SnapDB_GetOpenSlotOffset(void){
    size_t size;
    int16_t i;
    snapshot_t* dbstart = av_FileGetPtr(SnapDB, TI_APPVAR_TYPE, &size);
    size /= sizeof(snapshot_t);
    for(i = 0; i < size; i++){
        snapshot_t* dbcurr = &dbstart[i];
        if(!dbcurr->type) return i;
    }
    return i;
}

int16_t av_SnapDB_GetSlotMatchOffset(progname_t* program){
    size_t size;
    int16_t i;
    snapshot_t* dbstart = av_FileGetPtr(SnapDB, TI_APPVAR_TYPE, &size);
    size /= sizeof(snapshot_t);
    for(i = 0; i < size; i++){
        snapshot_t* dbcurr = &dbstart[i];
        if((!strncmp(dbcurr->fname, program->name, 8)) && (dbcurr->type == program->type))
            return i;
    }
    return -1;
}

snapshot_t* av_SnapDB_IndexToPtr(int16_t index){
    size_t size;
    snapshot_t* dbstart = av_FileGetPtr(SnapDB, TI_APPVAR_TYPE, &size);
    if((index == -1) ||(size == 0)) return 0;
    return &dbstart[index];
    
}

