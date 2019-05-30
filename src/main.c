//--------------------------------------
// Program Name:
// Author:
// License:
// Description:
//--------------------------------------

/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>


/* Standard headers - it's recommended to leave them included */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>

/* Other available headers */
// stdarg.h, setjmp.h, assert.h, ctype.h, float.h, iso646.h, limits.h, errno.h, debug.h
#define FORCE_INTERRUPTS
#include <graphx.h>
#include <fileioc.h>
#include <compression.h>
#include <intce.h>

#include "timestamps.h"
#include "gfx/all_gfx.h"
#include "crypto.h"
#include "indexing.h"
#include "menuopts.h"
#include "avfuncs.h"
#include <debug.h>

/* Put your function prototypes here */

void pgrm_EraseContent(void);
void pgrm_DrawBackground(gfx_sprite_t *icon);


int text_GetCenterX(char* string, int width);
uint24_t num_len(uint24_t num);
int progsort(const void* a, const void* b);
void av_SendAllToArchive(void);
void scrn_RenderRespIcon(char returncode, gfx_sprite_t* op_fail, gfx_sprite_t* op_success);
void exitProg(void);

/* Put all your globals here. */
const char *ProgName = "Blast";
const char *SubName = "File Integrity Software";
const char *Version = "2.0b";

/* Supporting Files */

// _OSSIZE = read this for OS size

#define ui_textstart_y 75
#define _OSSIZE (void**)0x020104
#define os_start (char*)0x020000

const char strings[][14] = {"File Options", "System Scans", "Snapshots", "Settings", "About", "Quit"};
const char desc[][60] = {"View and modify file data.",
    "Scan all RAM, Archive, or OS sectors.",
    "Change how this program works.",
    "View information about this program.",
    "Exit the program."};


void main(void) {
    bool progRun = true, firstLoop = true;
    char screen = MAIN;
    progname_t* prognames = NULL;
    snapname_t* snapnames = NULL;
    ti_var_t propfile;
    uint24_t num_programs = av_GetNumFiles();
    uint16_t num_snaps = av_GetSnapsCount();
    uint16_t snapopt_max = 0;
    ossave_t ossave = {0};
    uint24_t snapMemUse = 0;
    gfx_sprite_t* logo = gfx_MallocSprite(blast_icon_width, blast_icon_height);
    gfx_sprite_t* integ_pass = gfx_MallocSprite(integ_pass_icon_width, integ_pass_icon_height);
    gfx_sprite_t* integ_fail = gfx_MallocSprite(integ_fail_icon_width, integ_fail_icon_height);
    gfx_sprite_t* wait_icon = gfx_MallocSprite(waiticon_width, waiticon_height);
    gfx_sprite_t* warning = gfx_MallocSprite(warning_width, warning_height);
    gfx_sprite_t* op_fail = gfx_MallocSprite(fail_icon_width, fail_icon_height);
    gfx_sprite_t* op_success = gfx_MallocSprite(success_icon_width, success_icon_height);
    selected_t selected = {0};
    settings_t s = {0};
    ti_var_t settfile;
    //allocate memory
   // int_Disable();
    zx7_Decompress(warning, warning_compressed);
    zx7_Decompress(logo, blast_icon_compressed);
    zx7_Decompress(integ_pass, integ_pass_icon_compressed);
    zx7_Decompress(integ_fail, integ_fail_icon_compressed);
    zx7_Decompress(op_fail, fail_icon_compressed);
    zx7_Decompress(op_success, success_icon_compressed);
    ti_CloseAll();
    if(propfile = ti_Open(AVSettings, "r")){
        ti_Read(&s, sizeof(settings_t), 1, propfile);
        ti_Close(propfile);
    }
    else if(propfile = ti_Open(AVSettings, "w")) {
        ti_Close(propfile);
        s.maxSnaps = 20;
    }
    if(!(propfile = ti_Open(PropDB, "r+")))
        if(!(propfile = ti_Open(PropDB, "w+"))){
            exitProg();
            return;
        }
    ti_Close(propfile);
    prognames = (progname_t*)calloc(num_programs, sizeof(progname_t));
    if(num_snaps) snapnames = (snapname_t*)calloc(num_snaps, sizeof(snapname_t));
    if(prognames == NULL) {
        exitProg();
        return;
    }
    ti_Close(propfile);
    gfx_Begin();
    gfx_SetTextTransparentColor(1);
    gfx_SetTextBGColor(1);
    gfx_SetDrawBuffer();
    // loop save names of all files on device
    av_GenerateFileIndex(prognames, num_programs, &s);
    av_GenerateSnapIndex(snapnames, num_snaps, &snapMemUse);
    {
        stime_t* time;
        size_t temp;
        if(time = av_FileGetPtr(AVDefs, TI_APPVAR_TYPE, &temp))
            av_CheckRestoreTime(time);
    }
    // decompress all graphics
    do {
        unsigned char key = os_GetCSC();
        uint24_t progheap, heapoffset;
        bool refresh = firstLoop;
        progsave_t* current;
        progname_t* prog;
        snapname_t *snap;
        size_t size;
        exit_t e = {0};
        if(key == sk_Down){
            switch(screen){
                case MAIN:
                    if(selected.menu < QUIT) selected.menu++;
                    break;
                case FILE_OPTS:
                    if(!selected.progopt) {
                        if(selected.program < (num_programs-1)) selected.program++;
                    }else{
                        if(selected.progopt < SNAP_ENABLE) selected.progopt++;}
                    break;
                case SNAPSHOTS:
                    if(!selected.snapnum_opt){
                        if(selected.snapshot < (num_snaps - 1)) selected.snapshot++;}
                    else{
                        if(selected.snapnum_sel < snapopt_max) selected.snapnum_sel++;}
                    break;
                case SYS_SCANS:
                    if(selected.sysopt < 2) selected.sysopt++;
                    break;
                case SETTINGS:
                    if(selected.settopt < 1) selected.settopt++;
                    break;
            }
        }
        
        if(key == sk_Right){
            switch(screen){
                case FILE_OPTS:
                    if((selected.program + 13) < num_programs) selected.program += 13;
                    break;
                case SETTINGS:
                    if(selected.settopt == 1) s.maxSnaps++;
                    break;
            }
        }
        if(key == sk_Left){
            switch(screen){
                case FILE_OPTS:
                    if((selected.program - 13) >= 0) selected.program -= 13;
                    break;
                case SETTINGS:
                    if(selected.settopt == 1)
                        if(s.maxSnaps > 0) s.maxSnaps--;
                    break;
            }
        }
        if(key == sk_Up){
            switch(screen){
                case MAIN:
                    if(selected.menu > 0) selected.menu--;
                    break;
                case FILE_OPTS:
                    if(!selected.progopt) {
                        if(selected.program > 0) selected.program--;
                    }else{
                        if(selected.progopt > 1) selected.progopt--;}
                    break;
                case SNAPSHOTS:
                    if(!selected.snapnum_opt){
                        if(selected.snapshot > 0) selected.snapshot--;}
                    else{
                        if(selected.snapnum_sel > 0) selected.snapnum_sel--;}
                    break;
                case SYS_SCANS:
                    if(selected.sysopt > 0) selected.sysopt--;
                    break;
                case SETTINGS:
                    if(selected.settopt > 0) selected.settopt--;
                    break;
            }
        }
        if(key == sk_Stat){
            progname_t* newmem;
            num_programs = av_GetNumFiles();
            newmem = realloc(prognames, num_programs * sizeof(progname_t));
            if(newmem) prognames = newmem;
            memset(prognames, 0, num_programs * sizeof(progname_t));
            av_GenerateFileIndex(prognames, num_programs, &s);
        }
        
        if(key == sk_Enter) {
            switch(screen){
                case MAIN:
                    screen = selected.menu;
                    break;
                case FILE_OPTS:
                    switch(selected.progopt){
                        case TRACK_TOGG:
                            e.tracktogg = av_TogglePropTrack(&prognames[selected.program]);
                            break;
                        case TRACK_UPD:
                            e.trackupd = av_UpdateAttributes(&prognames[selected.program]);
                            break;
                        case SCAN:
                        {
                            progname_t* prog = &prognames[selected.program];
                            size_t size;
                            char* progstart = (char*)av_FileGetPtr(prog->name, prog->type, &size);
                            e.avscan = av_ScanData(progstart, size, warning);
                            break;
                        }
                        case SNAP_ENABLE:
                            if(av_FindSnap(snapnames, num_snaps, &prognames[selected.program]) == 'EOF'){
                                snapname_t* newmem = NULL;
                                e.snapcreate = av_CreateSnapshot(snapnames, num_snaps, &prognames[selected.program]);
                                num_snaps = av_GetSnapsCount();
                                snapnames = realloc(snapnames, num_snaps * sizeof(snapname_t));
                                if(!num_snaps) snapnames = NULL;
                                else {
                                    memset(snapnames, '\0', num_snaps * sizeof(snapname_t));
                                    av_GenerateSnapIndex(snapnames, num_snaps, &snapMemUse);
                                }
                            }
                            else
                                e.snapupd = av_UpdateSnapshot(snapnames, num_snaps, &prognames[selected.program]);
                            break;
                        default:
                            selected.progopt = 1;
                            
                    }
                    break;
                case SNAPSHOTS:
                    if(!selected.snapnum_opt) selected.snapnum_opt = 1;
                    else {
                        if(selected.snapnum_opt == 1){
                            if(selected.snapnum_sel == snapopt_max){
                                snapname_t* newmem = NULL;
                                e.snapdel = av_DeleteSnapshot(snap);
                                num_snaps = av_GetSnapsCount();
                                snapnames = realloc(snapnames, num_snaps * sizeof(snapname_t));
                                if(!num_snaps) {
                                    snapnames = NULL;
                                    snapMemUse = 0;
                                }
                                else {
                                    memset(snapnames, '\0', num_snaps * sizeof(snapname_t));
                                    av_GenerateSnapIndex(snapnames, num_snaps, &snapMemUse);
                                }
                                selected.snapnum_sel = 0;
                                selected.snapnum_opt = 0;
                                selected.snapshot = 0;
                            }
                            else e.snaprevert = av_RestoreSnapshot(snap, selected.snapnum_sel);
                        }
                        else selected.snapnum_opt = 1;
                        
                    }
                    break;
                case SETTINGS:
                    if(selected.settopt == 0)
                        s.indexSplit = (!s.indexSplit);
                    break;
                case SYS_SCANS:
                    if(selected.sysopt == 0) av_ChecksumOS(&ossave, os_start, *_OSSIZE);
                    if((selected.sysopt == 1) && (ossave.checksum)) e.ossave = av_SaveOSAttr(&ossave);
                    if(selected.sysopt == 2) {
                        char* os_end = *_OSSIZE;
                        size_t os_size = (size_t)(os_end - os_start);
                        e.avscan = av_ScanData(os_start, os_size, warning);
                    }
                    break;
            }
        }
        
        if(key == sk_Clear){
            switch(screen){
                case MAIN:
                    progRun = false;
                    break;
                case FILE_OPTS:
                    if(selected.progopt) selected.progopt = 0;
                    else screen = MAIN;
                    break;
                case SNAPSHOTS:
                    if(selected.snapnum_opt) selected.snapnum_opt = 0;
                    else screen = MAIN;
                    break;
                case SETTINGS:
                {
                    ti_var_t fp = ti_Open(AVSettings, "r+");
                    ti_Write(&s, sizeof(settings_t), 1, fp);
                    ti_Close(fp);
                }
                    screen = MAIN;
                    break;
                default:
                    screen = MAIN;
            }
        }
            
           
        if(key) refresh = true;
        if(refresh) {
            pgrm_DrawBackground(logo);
            switch(screen){
                case MAIN:
                {
                    int i;
                    stime_t* time;
                    size_t size;
                    gfx_SetTextFGColor(0);
                    gfx_SetColor(40); gfx_FillRectangle(5, selected.menu * 25 + 74, 130, 22);
                    for(i = 0; i < NUM_SCREENS; i++){
                        gfx_SetColor(0);
                        gfx_FillRectangle(15, i * 25 + ui_textstart_y, 110, 20);
                        gfx_SetColor(172);
                        gfx_FillRectangle(17, i * 25 + ui_textstart_y + 2, 106, 16);
                        gfx_PrintStringXY(strings[i], 17 + text_GetCenterX(strings[i], 110), i * 25 + ui_textstart_y + 6);
                    }
                    //gfx_PrintStringXY(desc[selected.menu], text_GetCenterX(desc[selected.menu], 100), 205);
                    gfx_PrintStringXY("BlastFIS v", 150, ui_textstart_y + 10);
                    gfx_PrintString(Version);
                    gfx_PrintStringXY("Your go-to for the", 150, ui_textstart_y + 25);
                    gfx_PrintStringXY("security of your", 150, ui_textstart_y + 35);
                    gfx_PrintStringXY("TI-84+ CE calculator", 150, ui_textstart_y + 45);
                    gfx_PrintStringXY("Malware Defintions: ", 150, ui_textstart_y + 60);
                    gfx_SetTextXY(160, ui_textstart_y + 70);
                    if(time = av_FileGetPtr(AVDefs, TI_APPVAR_TYPE, &size)){
                        char time_string[11] = {'\0'};
                        sprintf(time_string, "%04u-%02u-%02u", time->year, time->month, time->day);
                        gfx_PrintString(time_string);
                    } else gfx_PrintString("uninstalled");
                    
                    break;
                }
                case FILE_OPTS:
                {
                    int i;
                    progname_t* prog;
                    char cs_string[11] = {'\0'};
                    int ui_progdata_out = 175;
                    progopt_t progopt_yvals[] = {
                        {0, 0},
                        {106, ui_textstart_y + 75},
                        {106, ui_textstart_y + 85},
                        {106, ui_textstart_y + 95},
                        {106, ui_textstart_y + 120}
                    };
                    pgrm_EraseContent();
                    gfx_SetTextScale(2,2);
                    gfx_PrintStringXY("FILE OPTIONS", 5, 75);
                    gfx_SetTextScale(1,1);
                    progheap = selected.program / 13 * 13;
                    for(i = progheap; i < (progheap + 13); i++){
                        heapoffset = i - progheap;
                        gfx_SetTextFGColor(0);
                        if(i < num_programs){
                            prog = &prognames[i];
                            if(i == selected.program){
                                gfx_SetColor(40); gfx_SetTextFGColor(255);
                                gfx_FillRectangle(3, heapoffset * 10 + ui_textstart_y + 18, 70, 11);
                            }
                            gfx_PrintStringXY(prog->name, 5, heapoffset * 10 + ui_textstart_y + 20);
                        }
                    }
                    gfx_SetTextFGColor(0);
                    prog = &prognames[selected.program];
                    if(s.indexSplit && (!prog->indexed)) av_TellAttributes(prog);
                    if(selected.progopt){
                        int x = progopt_yvals[selected.progopt].x;
                        int y = progopt_yvals[selected.progopt].y - 1;
                        gfx_SetColor(140);
                        gfx_FillRectangle(x, y, 150, 10);
                    }
                    gfx_PrintStringXY("File Name: ", 100, ui_textstart_y + 20);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    gfx_PrintString(prog->name/*, 10, 9 * i + 75*/);
                    gfx_PrintStringXY("File Type: ", 100, ui_textstart_y + 30);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    if(prog->type == TI_PRGM_TYPE || prog->type == TI_PPRGM_TYPE)
                        gfx_PrintString("Program");
                    else gfx_PrintString("AppVar");
                    gfx_PrintStringXY("Attr Tracking: ", 100, ui_textstart_y + 45);
                    if(current = av_LocateFileInPropDB(prog)){
                        gfx_PrintString("enabled");
                        gfx_PrintStringXY("Disable ", 110, ui_textstart_y + 75);
                        if(current->size == prog->size)
                            gfx_TransparentSprite(integ_pass, ui_progdata_out + 65, ui_textstart_y + 53);
                        else
                            gfx_TransparentSprite(integ_fail, ui_progdata_out + 65, ui_textstart_y + 53);
                        if(current->checksum == prog->checksum)
                            gfx_TransparentSprite(integ_pass, ui_progdata_out + 65, ui_textstart_y + 63);
                        else
                            gfx_TransparentSprite(integ_fail, ui_progdata_out + 65, ui_textstart_y + 63);
                    }
                    else {
                        gfx_PrintString("disabled");
                        gfx_PrintStringXY("Enable ", 110, ui_textstart_y + 75);
                    }
                    gfx_PrintString("Tracking");
                    gfx_PrintStringXY("File Size: ", 100, ui_textstart_y + 55);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    gfx_PrintUInt(prog->size, num_len(prog->size));
                    gfx_PrintStringXY("CRC-32 CS: ", 100, ui_textstart_y + 65);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    sprintf(cs_string, "%xh", prog->checksum);
                    gfx_PrintString(cs_string);
                    gfx_PrintStringXY("Update Attributes", 110, ui_textstart_y + 85);
                    gfx_PrintStringXY("Scan File", 110, ui_textstart_y + 95);
                    if(e.avscan == FAIL) scrn_RenderRespIcon(FAIL, op_fail, op_success);
                   // gfx_SetTextXY(ui_progdata_out + 30, gfx_GetTextY());
                    gfx_PrintStringXY("Snapshot: ", 100, ui_textstart_y + 110);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    if(av_FindSnap(snapnames, num_snaps, prog) != 'EOF'){
                         gfx_PrintString("Enabled");
                         gfx_PrintStringXY("Update ", 110, ui_textstart_y + 120);
                    }
                     else {
                         gfx_PrintString("none");
                         gfx_PrintStringXY("Create ", 110, ui_textstart_y + 120);
                     }
                    gfx_PrintString("Snapshot");
                    scrn_RenderRespIcon(e.snapcreate, op_fail, op_success);
                    scrn_RenderRespIcon(e.snapupd, op_fail, op_success);
                    break;
                }
                case SYS_SCANS:
                {
                    ti_var_t osfile;
                    char cs_string[11] = {'\0'};
                    int ui_progdata_out = 140;
                    char* os_end = *_OSSIZE;
                    size_t os_size = (size_t)(os_end - os_start);
                    ossave.size = os_size;
                    pgrm_EraseContent();
                    gfx_SetTextScale(2,2);
                    gfx_PrintStringXY("SYSTEM SCANS", 5, 75);
                    gfx_SetTextScale(1,1);
                    gfx_PrintStringXY("Device Operating System", 50, ui_textstart_y + 30);
                    gfx_PrintStringXY("Size: ", 60, ui_textstart_y + 40);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    gfx_PrintUInt(os_size, num_len(os_size));
                    gfx_PrintStringXY("CRC-32 CS: ", 60, ui_textstart_y + 50);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    if(ossave.checksum){
                        progname_t prog;
                        progsave_t* saved;
                        gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                        strncpy(prog.name, OSSaveName, 8);
                        prog.type = TI_OSSAVE_TYPE;
                        sprintf(cs_string, "%xh", ossave.checksum);
                        gfx_PrintString(cs_string);
                        if(saved = av_LocateFileInPropDB(&prog)){
                            if(saved->checksum == ossave.checksum)
                                gfx_TransparentSprite(integ_pass, ui_progdata_out + 65, ui_textstart_y + 49);
                            else
                                gfx_TransparentSprite(integ_fail, ui_progdata_out + 65, ui_textstart_y + 49);
                        }
                    }
                    gfx_SetColor(140);
                    gfx_FillRectangle(65, (selected.sysopt * 10) + ui_textstart_y + 59, 140, 10);
                    gfx_PrintStringXY("Checksum OS", 70, ui_textstart_y + 60);
                    gfx_PrintStringXY("Save OS Attributes", 70, ui_textstart_y + 70);
                    scrn_RenderRespIcon(e.ossave, op_fail, op_success);
                    gfx_PrintStringXY("Scan OS", 70, ui_textstart_y + 80);
                    if(e.avscan == FAIL) scrn_RenderRespIcon(FAIL, op_fail, op_success);
                }
                    break;
                case SNAPSHOTS:
                {
                    int i;
                    int ui_progdata_out = 175;
                    progopt_t progopt_yvals[] = {
                        {0, 0},
                        {106, ui_textstart_y + 75},
                        {106, ui_textstart_y + 85},
                        {106, ui_textstart_y + 95},
                        {106, ui_textstart_y + 120}
                    };
                    pgrm_EraseContent();
                    gfx_SetTextScale(2,2);
                    gfx_PrintStringXY("SNAPSHOTS", 5, 75);
                    gfx_SetTextScale(1,1);
                    if(snapnames){
                        progheap = selected.snapshot / 13 * 13;
                        for(i = progheap; i < (progheap + 13); i++){
                            heapoffset = i - progheap;
                            gfx_SetTextFGColor(0);
                            if(i < num_snaps){
                                snap = &snapnames[i];
                                if(i == selected.snapshot){
                                    gfx_SetColor(40); gfx_SetTextFGColor(255);
                                    gfx_FillRectangle(3, heapoffset * 10 + ui_textstart_y + 18, 70, 11);
                                }
                                gfx_PrintStringXY(snap->progname, 5, heapoffset * 10 + ui_textstart_y + 20);
                            }
                        }
                        gfx_SetTextFGColor(0);
                        snap = &snapnames[selected.snapshot];
                  /*  if(selected.progopt){
                        int x = progopt_yvals[selected.progopt].x;
                        int y = progopt_yvals[selected.progopt].y - 1;
                        gfx_SetColor(140);
                        gfx_FillRectangle(x, y, 150, 10);
                    } */
                        gfx_PrintStringXY("File Name: ", 100, ui_textstart_y + 20);
                        gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                        gfx_PrintString(snap->progname/*, 10, 9 * i + 75*/);
                        gfx_PrintStringXY("File Type: ", 100, ui_textstart_y + 30);
                        gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                        if(snap->progtype == TI_PRGM_TYPE || snap->progtype == TI_PPRGM_TYPE)
                            gfx_PrintString("Program");
                        else gfx_PrintString("AppVar");
                        gfx_PrintStringXY("Snapshot Versions:", 100, ui_textstart_y + 50);
                        {
                            char* snapend = (char*)av_FileGetEnd(snap->snapname, TI_APPVAR_TYPE);
                            size_t size;
                            uint24_t snapopt_cur = 0;
                            char* snapcurr = (char*)av_FileGetPtr(snap->snapname, TI_APPVAR_TYPE, &size);
                            int texty = 65;
                            if(selected.snapnum_opt){
                                int ystart = ui_textstart_y + texty + (10 * selected.snapnum_sel) - 1;
                                gfx_SetColor(140);
                                gfx_FillRectangle(105, ystart, 140, 10);
                            }
                            snapopt_max = 0;
                            while(snapcurr < snapend){
                                snapshot_t *curr = (snapshot_t*)snapcurr;
                                char timestring[11] = {'\0'};
                                sprintf(timestring, "%04u-%02u-%02u", curr->time.year, curr->time.month, curr->time.day);
                                gfx_PrintStringXY(timestring, 110, ui_textstart_y + texty);
                                gfx_PrintStringXY("Revert", 190, ui_textstart_y + texty);
                                if(snapopt_cur == selected.snapnum_sel)
                                    scrn_RenderRespIcon(e.snaprevert, op_fail, op_success);
                                texty += 10;
                                snapopt_max++;
                                snapcurr = curr->data + curr->size;
                                snapopt_cur++;
                            }
                            gfx_PrintStringXY("Delete Snapshots", 110, ui_textstart_y + texty);
                        }
                    }
                    else gfx_PrintStringXY("No Snapshots Found!", 100, ui_textstart_y + 20);
            /*
                    gfx_PrintStringXY("File Size: ", 100, ui_textstart_y + 55);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    gfx_PrintUInt(prog->size, num_len(prog->size));
                    gfx_PrintStringXY("CRC-32 CS: ", 100, ui_textstart_y + 65);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    sprintf(cs_string, "%xh", prog->checksum);
                    gfx_PrintString(cs_string);
                    gfx_PrintStringXY("Update Attributes", 110, ui_textstart_y + 85);
                    gfx_PrintStringXY("Scan File", 110, ui_textstart_y + 95);
                    // gfx_SetTextXY(ui_progdata_out + 30, gfx_GetTextY());
                    gfx_PrintStringXY("Snapshot: ", 100, ui_textstart_y + 110);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    if(av_FindSnap(snapnames, num_snaps, prog)){
                        gfx_PrintString("Enabled");
                        gfx_PrintStringXY("Update ", 110, ui_textstart_y + 120);
                    }
                    else {
                        gfx_PrintString("none");
                        gfx_PrintStringXY("Create ", 110, ui_textstart_y + 120);
                    }
                    gfx_PrintString("Snapshot");
             */
                    break;
                }
                case SETTINGS:
                {
                    int snap_x;
                    progopt_t settings_yvals[] = {
                        {25, ui_textstart_y + 22},
                        {65, ui_textstart_y + 65}
                    };
                    pgrm_EraseContent();
                    gfx_SetTextScale(2,2);
                    gfx_PrintStringXY("AV SETTINGS", 5, 75);
                    gfx_SetTextScale(1,1);
                    gfx_SetColor(140);
                    gfx_FillRectangle(settings_yvals[selected.settopt].x, settings_yvals[selected.settopt].y - 2, 200, 11);
                    gfx_SetColor(0);
                    gfx_Rectangle(10, ui_textstart_y + 20, 11, 11);
                    if(s.indexSplit) {
                        gfx_SetTextXY(12, ui_textstart_y + 22);
                        gfx_PrintChar('X');
                    }
                    
                    gfx_PrintStringXY("Enable Split Indexing Mode", 30, ui_textstart_y + 22);
                    gfx_PrintStringXY("Reserve checksum and size calculations", 40, ui_textstart_y + 32);
                    gfx_PrintStringXY("for first viewing in Program Options.", 40, ui_textstart_y + 42);
                    gfx_PrintStringXY("Less load time, slight scroll lag.", 40, ui_textstart_y + 52);
                    gfx_SetTextXY(10, ui_textstart_y + 65);
                    gfx_PrintUInt(num_snaps, num_len(num_snaps));
                    gfx_PrintString(" / ");
                    gfx_PrintUInt(s.maxSnaps, num_len(s.maxSnaps));
                    gfx_PrintStringXY("Snapshot Max File Count", 70, gfx_GetTextY());
                    gfx_PrintStringXY("Memory Usage: ", 20, ui_textstart_y + 77);
                    gfx_PrintUInt(snapMemUse, num_len(snapMemUse));
                    gfx_PrintString(" bytes");
                    
                    
                }
                    break;
                case QUIT:
                    progRun = false;
                    break;
                default:
                    pgrm_EraseContent();
                    gfx_PrintStringXY("Option Not Implemented", 10, 75);
            }
        }
        gfx_BlitBuffer();
        firstLoop = false;
    } while(progRun);
    
	/* Fill in the body of the main function here */
    free(logo);
    free(prognames);
    free(integ_pass);
    free(integ_fail);
    free(wait_icon);
    free(warning);
    free(snapnames);
    exitProg();
    return;
}

void exitProg(void){
    gfx_End();
    av_SendAllToArchive();
    pgrm_CleanUp();
    os_ClrHomeFull();
}

void av_SendAllToArchive(void){
    char *var_name;
    uint8_t *search_pos = NULL;
    uint8_t type;
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL) {
        if (type == TI_PRGM_TYPE || type == TI_PPRGM_TYPE || type == TI_APPVAR_TYPE){
            if((!strncmp(var_name, SnapDB, 8)) ||
               (!strncmp(var_name, PropDB, 8)) ||
               (!strncmp(var_name, AVSettings, 8)) ||
               (!strncmp(var_name, AVDefs, 8)) ||
               (!strncmp(var_name, OSProp, 8)) ||
               (!strncmp(var_name, "AVsh", 4))){
                    ti_var_t fp = ti_OpenVar(var_name, "r", type);
                    if(fp) ti_SetArchiveStatus(true, fp);
                    ti_Close(fp);
                }
        }
    }
}


void scrn_RenderRespIcon(char returncode, gfx_sprite_t* op_fail, gfx_sprite_t* op_success){
    if(returncode){
        if(returncode == SUCCESS)
            gfx_TransparentSprite(op_success, gfx_GetTextX() + 10, gfx_GetTextY() - 1);
        if(returncode == FAIL)
            gfx_TransparentSprite(op_fail, gfx_GetTextX() + 10, gfx_GetTextY() - 1);
    }
    return;
}




uint24_t num_len(uint24_t num){
    uint24_t count = 0;
    if(num == 0) return 1;
    while(num != 0)
    {
        // n = n/10
        num /= 10;
        ++count;
    }
    return count;
}

int text_GetCenterX(char* string, int width){
    return (width - gfx_GetStringWidth(string)) / 2;
}


void pgrm_EraseContent(void){
    gfx_SetColor(205);
    gfx_FillRectangle(111, 71, 319-111, 219-71);
}

void pgrm_DrawBackground(gfx_sprite_t *icon){
    // draw background
    gfx_FillScreen(205);
    gfx_SetColor(0);
    gfx_FillRectangle(0, 0, 320, 70);
    gfx_SetColor(40);
    gfx_FillRectangle(4, 4, 320-8, 70-8);
    gfx_SetTextFGColor(255);
    gfx_SetTextScale(3,3);
    gfx_TransparentSprite(icon, 5, 5);
    gfx_PrintStringXY("B L A S T", 100, 20);
    gfx_SetTextScale(2,2);
    gfx_PrintStringXY("2", 265, 15);
    gfx_SetTextScale(1,1);
    gfx_PrintStringXY("TI File Integrity Software", 100, 50);
    gfx_SetTextFGColor(0);
    gfx_PrintStringXY("(c) 2019 - Anthony Cagliano, ClrHome", 5, 228);
}






// currently unused but might be needed
/*bool time_IsFileOutdated(time_struct_short_t *file, uint8_t maxAge){
    uint24_t systemdays, filedays;
    systemdays = (systemtime.year * 365) + (systemtime.month * 30) + systemtime.day;
    filedays = (file->year * 365) + (file->month * 30) + file->day;
    if(systemdays - filedays >= maxAge) return 1;
    return 0;
    
}
*/


