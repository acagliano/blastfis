
#include <string.h>
#include "indexing.h"
#include "crypto.h"


int av_GetNumFiles(){
    char* var_name;
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL)
        switch(type){
    case TI_PRGM_TYPE:
    case TI_PPRGM_TYPE:
    case TI_APPVAR_TYPE:
        if(!(!memcmp(var_name, "#", 8) || !memcmp(var_name, "!", 8))) num_programs++;
        break;
    }
}

int av_GenerateFileIndex(progname_t* prognames, int count){
    ti_var_t dbfile;
    char* var_name;
    progsave_t* db_data;
    size_t db_size;
    if(!(dbfile = ti_Open(PropDB, "r+"))) return 0;
    db_data = (progsave_t*)ti_GetDataPtr(dbfile);
    db_size = ti_GetSize(dbfile);
    ti_Close(dbfile);
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL){
        progsave_t* db_instance = db_data;
        size_t size_instance = db_size;
        switch(type){
            case TI_PRGM_TYPE:
            case TI_PPRGM_TYPE:
            case TI_APPVAR_TYPE:
                ti_Rewind();
                if(!(!memcmp(var_name, "#") || !memcmp(var_name, "!"))){
                    progname_t* prog = &prognames[num_programs];
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
                            db_instance++
                        }
                    }
                }
        }
    }
    qsort(prognames, num_programs, sizeof(progname_t), progsort);
    return 1;
}

progsave_t* av_LocateFileInDB(progname_t* program){
    ti_var_t dbfile;
    progsave_t* db_data;
    size_t db_size;
    if(!(dbfile = ti_Open(PropDB, "r+"))) return NULL;
    db_data = (progsave_t*)ti_GetDataPtr(dbfile);
    db_size = ti_GetSize(dbfile);
    ti_Close(dbfile);
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
    if(!(dbfile_read = ti_Open(PropDB, "r+"))) return;
    if(!(dbfile_write = ti_Open(PropDB, "r+"))) return;
    db_size = ti_GetSize(dbfile_read);
    db_offset = delete - db_size;
    ti_Seek(db_offset, SEEK_SET, dbfile_write);
    ti_Seek(db_offset + sizeof(progsave_t), SEEK_SET, dbfile_read);
    while(ti_Write(ti_GetDataPtr(dbfile_write), sizeof(progsave_t), 1, dbfile_read)){
        ti_Seek(sizeof(progsave_t), SEEK_CUR, dbfile_write);
    }
    ti_Close(dbfile_read);
    ti_Close(dbfile_write);
}
