
#include <string.h>
#include <debug.h>
#include <fileioc.h>
#include "indexing.h"
#include "crypto.h"
#include "avfuncs.h"

const char *PropDB = "AVPropDB";
const char *AvDB = "AVDefsDB";


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
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL)
        switch(type){
    case TI_PRGM_TYPE:
    case TI_PPRGM_TYPE:
    case TI_APPVAR_TYPE:
        if(!(!memcmp(var_name, "#", 8) || !memcmp(var_name, "!", 8))) count++;
        break;
    }
    return count;
}

int av_GenerateFileIndex(progname_t* prognames, int count){
    ti_var_t dbfile, openfile;
    int i;
    char* var_name;
    uint8_t *search_pos = NULL;
    size_t db_size;
    uint8_t type;
    progsave_t* db_data = av_FileGetPtr(PropDB, TI_APPVAR_TYPE, &db_size);
    progsave_t* db_end = db_data + db_size;
    dbg_sprintf(dbgout, "File Start: %p\n", db_data);
    dbg_sprintf(dbgout, "File End: %p\n", db_end);
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL){
        progsave_t* db_instance = db_data;
        switch(type){
            case TI_PRGM_TYPE:
            case TI_PPRGM_TYPE:
            case TI_APPVAR_TYPE:
                if(!(!memcmp(var_name, "#", 8) || !memcmp(var_name, "!", 8))){
                    progname_t* prog = &prognames[i++];
                    prog->type = type;
                    memcpy(prog->name, var_name, 8);
                    if(openfile = ti_OpenVar(prog->name, "r", type)){
                        int value = 0; unsigned long checksum = 0;
                        progsave_t read;
                        prog->size = ti_GetSize(openfile);
                        prog->checksum = rc_crc32(0, ti_GetDataPtr(openfile), prog->size);
                        ti_Close(openfile);
                        dbg_sprintf(dbgout, "Checking address: %p\n", db_instance);
                        dbg_sprintf(dbgout, "Ending address: %p\n", db_end);
                        while(db_instance < db_end){
                            dbg_sprintf(dbgout, "Checking address: %p\n", db_instance);
                            if(!memcmp(db_instance->name, var_name, 8)){
                                prog->prop_track = db_instance - db_data;
                                break;
                            }
                            db_instance++;
                        }
                    }
                }
        }
    }
    qsort(prognames, count, sizeof(progname_t), progsort);
    return 1;
}

progsave_t* av_LocateFileInDB(progname_t* program){
    ti_var_t dbfile;
    size_t db_size;
    progsave_t* db_data = av_FileGetPtr(program->name, TI_APPVAR_TYPE, &db_size);
    while(db_size){
        if(!memcmp(db_data->name, program->name, 8)) return db_data;
        db_size -= sizeof(progsave_t);
        db_data++;
    }
    return NULL;
}

void av_CollapseDB(progsave_t* delete){
    ti_var_t dbfile_read, dbfile_write;
    size_t db_size, db_offset;
    progsave_t* collapse;
    if(!(dbfile_read = ti_Open(PropDB, "r+"))) return;
    if(!(dbfile_write = ti_Open(PropDB, "r+"))) return;
    db_size = ti_GetSize(dbfile_read);
    db_offset = (size_t)delete - db_size;
    ti_Seek(db_offset, SEEK_SET, dbfile_write);
    ti_Seek(db_offset + sizeof(progsave_t), SEEK_SET, dbfile_read);
    collapse = (progsave_t*)ti_GetDataPtr(dbfile_write);
    while(ti_Write(collapse, sizeof(progsave_t), 1, dbfile_read)) collapse++;
    ti_Close(dbfile_read);
    ti_Close(dbfile_write);
    av_ResizeFile(PropDB, TI_APPVAR_TYPE, -sizeof(progsave_t));
}

void* av_FileGetPtr(const char *name, uint8_t type, int *Len){
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
        if((fp_size = ti_Resize(sizealt, fp)) > 0){
            return fp_size;
        }
        ti_Close(fp);
    }
    return fp_size;
}


void av_TogglePropTrack(progname_t* program){
    if(program->prop_track == 0) enable_PropTrack(program);
    else disable_PropTrack(program);
}

void enable_PropTrack(progname_t* program){
    size_t db_size;
    progsave_t* db_data = av_FileGetPtr(PropDB, TI_APPVAR_TYPE, &db_size);
}

void disable_PropTrack(progname_t* program){
    progsave_t* ref;
    if(ref = av_LocateFileInDB(program)) av_CollapseDB(ref);
}
