
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
    if(!(dbfile = ti_Open(PropDB, "r+"))) return 1;
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
                if(!(!strcmp(var_name, "#") || !strcmp(var_name, "!"))){
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
                            db_instance += sizeof(progsave_t);
                        }
                    }
                }
        }
    }
    qsort(prognames, num_programs, sizeof(progname_t), progsort);
}
