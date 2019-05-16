
#include <string.h>
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
    progsave_t* db_data = av_FileGetPtr(PropDB, &db_size);
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL){
        progsave_t* db_instance = db_data;
        size_t size_instance = db_size;
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
                        while(size_instance){
                            if(!memcmp(db_instance->name, var_name, 8)){
                                prog->prop_track = db_instance - db_data;
                                break;
                            }
                            size_instance -= sizeof(progsave_t);
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
    progsave_t* db_data = av_FileGetPtr(program->name, &db_size);
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
}

void* av_FileGetPtr(char *name, int *Len){
    ti_var_t fp = ti_Open(name, "r+");
    void* addr;
    if(!fp) return NULL;
    *Len = ti_GetSize(fp);
    addr = ti_GetDataPtr(fp);
    ti_Close(fp);
    return addr;
}

void av_TogglePropTrack(progname_t* program){
    if(program->prop_track == 0) enable_PropTrack(program);
    else disable_PropTrack(program);
}

void enable_PropTrack(progname_t* program){
    ti_var_t dbfile;
    progsave_t temp = {0};      // zero temporary copy of program save
    if(dbfile = ti_Open(PropDB, "r+")){     // open file (will exist at this point and be either empty or not
        temp.type = program->type;              // save type to temp
        memcpy(temp.name, program->name, 8);   // copy name to temp
        temp.checksum = program->checksum;      // copy checksum to temp
        temp.size = program->size;              // copy size to temp
        ti_Seek(0, SEEK_END, dbfile);           // seek to end of file (since disable ensures deleted entries removed
        program->prop_track = ti_Tell(dbfile);      // save offset to location of item in program index
        ti_Write(&temp, sizeof(progsave_t), 1, dbfile); // write temp to the database file (should be appending)
        ti_Close(dbfile);               // close file  (Would opening in append mode perhaps be better here?)
    }
}

void disable_PropTrack(progname_t* program){
    progsave_t* ref;
    if(ref = av_LocateFileInDB(program)) av_CollapseDB(ref);
}