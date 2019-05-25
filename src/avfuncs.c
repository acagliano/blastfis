
#include <string.h>
#include <debug.h>
#include <fileioc.h>
#include <graphx.h>
#include "indexing.h"
#include "crypto.h"
#include "avfuncs.h"
#include "gfx/all_gfx.h"

const char *PropDB = "AVPropDB";
const char *AvDB = "AVDefsDB";
const char *SnapDB = "AVSnapDB";
const char *SnapFile = "AVshXXXX";
const char *AVSettings = "AVSett";
const char *OSProp = "AVOSProp";
const char *AVDefs = "AVMALDEF";

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

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

uint16_t av_GetSnapsCount(void){
    char* var_name;
    uint16_t count = 0;
    uint8_t *search_pos = NULL;
    while((var_name = ti_Detect(&search_pos, NULL)) != NULL){
        if(!strncmp(var_name, "AVsh", 4)) count++;
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

void av_GenerateSnapIndex(snapname_t* snapnames, uint24_t count){
    int i = 0;
    char* var_name;
    uint8_t *search_pos = NULL;
    uint8_t type;
    if(!count) return;
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL){
        unsigned int complete = (100 * i / count);
        gfx_SetColor(255);
        gfx_FillRectangle(0, 0, 320, 25);
        gfx_PrintStringXY("Indexing snapshots...", 5, 5);
        gfx_BlitBuffer();
        if(!strncmp(var_name, "AVsh", 4)){
            snapname_t* snap = &snapnames[i++];
            size_t size;
            snapshot_t* data = av_FileGetPtr(var_name, TI_APPVAR_TYPE, &size);
            strncpy(snap->snapname, var_name, 8);
            strncpy(snap->progname, data->name, 8);
            snap->progtype = data->type;
        }
    }
}

void av_TellAttributes(progname_t* program){
    ti_var_t openfile;
    if(openfile = ti_OpenVar(program->name, "r", program->type)){
        int value = 0; unsigned long checksum = 0;
        char* start = (char*)ti_GetDataPtr(openfile);
        program->prop_track = (unsigned int)av_LocateFileInPropDB(program);
        program->size = ti_GetSize(openfile);
        program->checksum = rc_crc32(0, ti_GetDataPtr(openfile), program->size);
        ti_Close(openfile);
        program->indexed = true;
    }
}

void av_ChecksumOS(ossave_t* ossave, char *start, char *end){
    size_t size = (size_t)(end - start);
    ossave->checksum = rc_crc32(0, start, size);
}

void av_SaveOSAttr(ossave_t* ossave){
    ti_var_t osdb;
    if(osdb = ti_Open(OSProp, "r+")){
        ossave_t* save = (ossave_t*)ti_GetDataPtr(osdb);
        save->checksum = ossave->checksum;
    }
    else {
        osdb = ti_Open(OSProp, "w");
        ti_Write(ossave, sizeof(ossave_t), 1, osdb);
    }
    ti_Close(osdb);
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


void av_DeleteSnapshot(snapname_t* snap){
    ti_Delete(snap->snapname);
}

void av_CreateSnapshot(snapname_t* snapnames, uint24_t num_snaps, progname_t* program){
    size_t size;
    uint16_t index = 0;
    char snapname[9] = {'\0'};
    settings_t* s = (settings_t*)av_FileGetPtr(AVSettings, TI_APPVAR_TYPE, &size);
    while(index < s->maxSnaps){
        ti_var_t snapfile;
        strncpy(snapname, SnapFile, 8);
        sprintf(snapname + 4, "%04x", index);
        if(!(snapfile = ti_Open(snapname, "r"))) break;
        ti_Close(snapfile);
        index++;
    }
    if(index == s->maxSnaps) return;
    {
        ti_var_t snapshot, srcfile;
        if((snapshot = ti_Open(snapname, "w")) &&
           (srcfile = ti_OpenVar(program->name, "r", program->type))){
            snapshot_t snapdata;
            snapdata.size = ti_GetSize(srcfile);
            strncpy(&snapdata.name, program->name, 8);
            snapdata.type = program->type;
            boot_GetDate(&snapdata.time.day, &snapdata.time.month, &snapdata.time.year);
            ti_Write(&snapdata, sizeof(snapshot_t) - 1, 1, snapshot);
            ti_Write(ti_GetDataPtr(srcfile), ti_GetSize(srcfile), 1, snapshot);
        }
        ti_Close(snapshot);
        ti_Close(srcfile);
    }
}

void av_UpdateSnapshot(snapname_t* snapnames, uint24_t count, progname_t* program){
    uint16_t index;
    if((index = av_FindSnap(snapnames, count, program)) != 'EOF'){
        snapname_t* snap = &snapnames[index];
        ti_var_t snapshot, srcfile;
        if((snapshot = ti_Open(snap->snapname, "r+")) &&
           (srcfile = ti_OpenVar(program->name, "r", program->type))){
            snapshot_t snapdata;
            snapdata.size = ti_GetSize(srcfile);
            strncpy(&snapdata.name, program->name, 8);
            snapdata.type = program->type;
            boot_GetDate(&snapdata.time.day, &snapdata.time.month, &snapdata.time.year);
            ti_Seek(0, SEEK_END, snapshot);
            ti_Write(&snapdata, sizeof(snapshot_t) - 1, 1, snapshot);
            ti_Write(ti_GetDataPtr(srcfile), ti_GetSize(srcfile), 1, snapshot);
        }
        ti_Close(snapshot);
        ti_Close(srcfile);
    }
}

void av_RestoreSnapshot(snapname_t* snap, uint24_t which){
    size_t size;
    ti_var_t file;
    char* snapdata = (char*)av_FileGetPtr(snap->snapname, TI_APPVAR_TYPE, &size);
    char* snapend = (char*)av_FileGetEnd(snap->snapname, TI_APPVAR_TYPE);
    while(which){
        snapshot_t *curr = (snapshot_t*)snapdata;
        snapdata = (char*)(&curr->data + curr->size);
        which--;
    }
        
    ti_DeleteVar(snap->progname, snap->progtype);
    if(file = ti_OpenVar(snap->progname, "w", snap->progtype)){
        snapshot_t *curr = (snapshot_t*)snapdata;
        ti_Write(curr->data, curr->size, 1, file);
        dbg_sprintf(dbgout, "Writing %u bytes.", curr->size);
        ti_Close(file);
    }
}


int24_t av_FindSnap(snapname_t* snapnames, size_t count, progname_t* program){
    uint16_t index = 0;
    size_t size;
    if(snapnames == NULL) return 'EOF';
    for(index = 0; index < count; index++){
        snapname_t* snap = &snapnames[index];
        ti_var_t file;
        if((!strncmp(program->name, snap->progname, 8)) && (program->type == snap->progtype))
            if(file = ti_Open(snap->snapname, "r")){
                ti_Close(file);
                return index;
            }
    }
    return 'EOF';
}

void gfx_PrintBoundedStringXY(const char* text, int len, int x, int y){
    int i;
    gfx_SetTextXY(x, y);
    for(i = 0; i < len; i++){
        if(text[i] == '\0') return;
        gfx_PrintChar(text[i]);
    }
    gfx_PrintString("...");
}

void av_ScanData(char* data, size_t size, gfx_sprite_t* warning){
    size_t avsize;
    char* filestart = (char*)av_FileGetPtr(AVDefs, TI_APPVAR_TYPE, &avsize);
    int l = 100, t = 75, w = (310 - l), h = (230 - t);
    int curx, cury;
    int resultx = 120;
    unsigned char len;
    stime_t timestamp;
    gfx_SetColor(40);
    gfx_FillRectangle(l, t, w, h);
    gfx_SetColor(205);
    gfx_FillRectangle(l + 2, t + 2, w - 4, h - 4);
    curx = l + 4; cury = t + 4;
    memcpy(&timestamp, filestart, sizeof(stime_t));
    filestart += sizeof(stime_t);
    avsize -= sizeof(stime_t);
    gfx_BlitBuffer();
    while(avsize){
        char disp[11];
        uint24_t locate;
        char readbuff[256] = {'\0'};
        len = *filestart;
        memcpy(readbuff, filestart + 1, len);
        gfx_PrintStringXY(readbuff, curx, cury);
        filestart += (len + 1);
        avsize -= (len + 1);
        len = *filestart;
        if(locate = av_FindScanMatch(data, size, filestart + 1, len)){
            char ptr_string[7];
            gfx_TransparentSprite(warning, curx + resultx, cury - 2);
            gfx_PrintStringXY(" 0x", curx + resultx + 15, cury);
            sprintf(ptr_string, "%02x", locate);
            gfx_PrintString(ptr_string);
        }
        else gfx_PrintStringXY("None found", curx + resultx, cury);
        filestart += (len + 1);
        avsize -= (len + 1);
        cury += 10;
        if(cury >= 210) {
            gfx_PrintStringXY("Any key to continue...", curx, cury);
            while(!os_GetCSC());
            gfx_FillRectangle(l + 2, t + 2, w - 4, h - 4);
            cury = t + 4;
        }
        gfx_BlitBuffer();
    }
    gfx_PrintStringXY("Done!", curx, cury);
    gfx_BlitBuffer();
    while(!os_GetCSC());
}


uint24_t av_FindScanMatch(char* data, size_t datasize, char* readbuff, size_t len){
    if(!len) return 0;
    while(datasize--){
        if(!memcmp(data, readbuff, len)) return (uint24_t)data;
        data++;
    }
    return 0;
}
