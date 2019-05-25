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
//#include <debug.h>

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
int num_len(int num);
int progsort(const void* a, const void* b);
void av_SendAllToArchive(void);

/* Put all your globals here. */
const char *ProgName = "Blast";
const char *SubName = "Calculator Security Suite";
const char *Version = "0.95b";

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
    gfx_sprite_t* logo = gfx_MallocSprite(blast_icon_width, blast_icon_height);
    gfx_sprite_t* integ_pass = gfx_MallocSprite(integ_pass_icon_width, integ_pass_icon_height);
    gfx_sprite_t* integ_fail = gfx_MallocSprite(integ_fail_icon_width, integ_fail_icon_height);
    gfx_sprite_t* wait_icon = gfx_MallocSprite(waiticon_width, waiticon_height);
    gfx_sprite_t* warning = gfx_MallocSprite(warning_width, warning_height);
    selected_t selected = {0};
    //allocate memory
   // int_Disable();
    zx7_Decompress(warning, warning_compressed);
    zx7_Decompress(logo, blast_icon_compressed);
    zx7_Decompress(integ_pass, integ_pass_icon_compressed);
    zx7_Decompress(integ_fail, integ_fail_icon_compressed);
    ti_CloseAll();
    if(!(propfile = ti_Open(AVSettings, "r+"))){
        if(propfile = ti_Open(AVSettings, "w")){
            settings_t s = {0};
            s.maxSnaps = 100;
            ti_Write(&s, sizeof(settings_t), 1, propfile);
            ti_Close(propfile);
        }
    }
    if(!(propfile = ti_Open(PropDB, "r+")))
        if(!(propfile = ti_Open(PropDB, "w+"))){
            os_ClrHomeFull();
            exit(1);
        }
    ti_Close(propfile);
    prognames = (progname_t*)calloc(num_programs, sizeof(progname_t));
    if(num_snaps) snapnames = (snapname_t*)calloc(num_snaps, sizeof(snapname_t));
    if(prognames == NULL) {
        os_ClrHomeFull();
        exit(1);
    }
    ti_Close(propfile);
    gfx_Begin();
    gfx_SetTextTransparentColor(1);
    gfx_SetTextBGColor(1);
    gfx_SetDrawBuffer();
    // loop save names of all files on device
    av_GenerateFileIndex(prognames, num_programs);
    av_GenerateSnapIndex(snapnames, num_snaps);
    // decompress all graphics
    do {
        unsigned char key = os_GetCSC();
        uint24_t progheap, heapoffset;
        bool refresh = firstLoop;
        progsave_t* current;
        progname_t* prog;
        snapname_t *snap;
        size_t size;
        settings_t* s = (settings_t*)av_FileGetPtr(AVSettings, TI_APPVAR_TYPE, &size);
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
                    dbg_sprintf(dbgout, "%u\n", snapopt_max);
                    dbg_sprintf(dbgout, "%u\n", selected.snapnum_sel);
                    if(!selected.snapnum_opt){
                        if(selected.snapshot < (num_snaps - 1)) selected.snapshot++;}
                    else{
                        if(selected.snapnum_sel < snapopt_max) selected.snapnum_sel++;}
                    break;
                case SYS_SCANS:
                    if(selected.sysopt < 2) selected.sysopt++;
                    break;
            }
        }
        
        if(key == sk_Right){
            switch(screen){
                case FILE_OPTS:
                    if((selected.program + 13) < num_programs) selected.program += 13;
                    break;
            }
        }
        if(key == sk_Left){
            switch(screen){
                case FILE_OPTS:
                    if((selected.program - 13) >= 0) selected.program -= 13;
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
            }
        }
        if(key == sk_Stat){
            progname_t* newmem;
            num_programs = av_GetNumFiles();
            newmem = realloc(prognames, num_programs * sizeof(progname_t));
            if(newmem) prognames = newmem;
            memset(prognames, 0, num_programs * sizeof(progname_t));
            av_GenerateFileIndex(prognames, num_programs);
        }
        
        if(key == sk_Enter) {
            if(screen == MAIN) screen = selected.menu;
            else if(screen == FILE_OPTS){
                if(!selected.progopt) selected.progopt = 1;
                else if(selected.progopt == TRACK_TOGG)
                    av_TogglePropTrack(&prognames[selected.program]);
                else if(selected.progopt == TRACK_UPD)
                    av_UpdateAttributes(&prognames[selected.program]);
                else if(selected.progopt == SCAN){
                    progname_t* prog = &prognames[selected.program];
                    size_t size;
                    char* progstart = (char*)av_FileGetPtr(prog->name, prog->type, &size);
                    av_ScanData(progstart, size, warning);
                }
                else if(selected.progopt == SNAP_ENABLE){
                    if(av_FindSnap(snapnames, num_snaps, &prognames[selected.program]) == 'EOF'){
                        snapname_t* newmem = NULL;
                        av_CreateSnapshot(snapnames, num_snaps, &prognames[selected.program]);
                        num_snaps = av_GetSnapsCount();
                        snapnames = realloc(snapnames, num_snaps * sizeof(snapname_t));
                        if(!num_snaps) snapnames = NULL;
                        else {
                            memset(snapnames, '\0', num_snaps * sizeof(snapname_t));
                            av_GenerateSnapIndex(snapnames, num_snaps);
                        }
                    }
                    else
                         av_UpdateSnapshot(snapnames, num_snaps, &prognames[selected.program]);
                }
            }
            else if(screen == SNAPSHOTS){
                if(!selected.snapnum_opt) selected.snapnum_opt = 1;
                else {
                    if(selected.snapnum_opt == 1){
                        if(selected.snapnum_sel == snapopt_max){
                            snapname_t* newmem = NULL;
                            av_DeleteSnapshot(snap);
                            num_snaps = av_GetSnapsCount();
                            snapnames = realloc(snapnames, num_snaps * sizeof(snapname_t));
                            if(!num_snaps) snapnames = NULL;
                            else {
                                memset(snapnames, '\0', num_snaps * sizeof(snapname_t));
                                av_GenerateSnapIndex(snapnames, num_snaps);
                            }
                            selected.snapnum_sel = 0;
                            selected.snapnum_opt = 0;
                        }
                        else av_RestoreSnapshot(snap, selected.snapnum_sel);
                    }
                    else selected.snapnum_opt = 1;
                        
                }
            }
            else if(screen == SETTINGS){
                size_t size;
                settings_t* s = (settings_t*)av_FileGetPtr(AVSettings, TI_APPVAR_TYPE, &size);
                if(selected.settopt == 0) s->indexSplit = (!s->indexSplit);
            }
            else if(screen == SYS_SCANS){
                if(selected.sysopt == 0) av_ChecksumOS(&ossave, os_start, *_OSSIZE);
                if((selected.sysopt == 1) && (ossave.checksum)) av_SaveOSAttr(&ossave);
                if(selected.sysopt == 2) {
                    char* os_end = *_OSSIZE;
                    size_t os_size = (size_t)(os_end - os_start);
                    av_ScanData(os_start, os_size, warning);
                }
            }
        }
        
        if(key == sk_Clear){
            if(screen == MAIN) progRun = false;
            else if(screen == FILE_OPTS){
                if(selected.progopt) selected.progopt = 0;
                else screen = MAIN;
            }
            else if(screen == SNAPSHOTS){
                if(selected.snapnum_opt) selected.snapnum_opt = 0;
                else screen = MAIN;
            }
            else screen = MAIN;
        }
        if(key) refresh = true;
        if(refresh) {
            pgrm_DrawBackground(logo);
            switch(screen){
                case MAIN:
                {
                    int i;
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
                    if(s->indexSplit && (!prog->indexed)) av_TellAttributes(prog);
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
                        gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                        sprintf(cs_string, "%xh", ossave.checksum);
                        gfx_PrintString(cs_string);
                        if(osfile = ti_Open(OSProp, "r")){
                            ossave_t* saved = (ossave_t*)ti_GetDataPtr(osfile);
                            if(saved->checksum == ossave.checksum)
                                gfx_TransparentSprite(integ_pass, ui_progdata_out + 65, ui_textstart_y + 49);
                            else
                                gfx_TransparentSprite(integ_fail, ui_progdata_out + 65, ui_textstart_y + 49);
                            ti_Close(osfile);
                        }
                    }
                    gfx_SetColor(140);
                    gfx_FillRectangle(65, (selected.sysopt * 10) + ui_textstart_y + 59, 140, 10);
                    gfx_PrintStringXY("Checksum OS", 70, ui_textstart_y + 60);
                    gfx_PrintStringXY("Save OS Attributes", 70, ui_textstart_y + 70);
                    gfx_PrintStringXY("Scan OS", 70, ui_textstart_y + 80);
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
                                sprintf(timestring, "%04u:%02u:%02u", curr->time.year, curr->time.month, curr->time.day);
                                gfx_PrintStringXY(timestring, 110, ui_textstart_y + texty);
                                gfx_PrintStringXY("Revert", 190, ui_textstart_y + texty);
                                texty += 10;
                                snapopt_max++;
                                snapcurr = (char*)(&curr->data + curr->size);
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
                    progopt_t settings_yvals[] = {
                        {25, ui_textstart_y + 20},
                    };
                    pgrm_EraseContent();
                    gfx_SetTextScale(2,2);
                    gfx_PrintStringXY("AV SETTINGS", 5, 75);
                    gfx_SetTextScale(1,1);
                    gfx_SetColor(140);
                    gfx_FillRectangle(25, settings_yvals[selected.settopt].y - 1, 200, 11);
                    gfx_SetColor(0);
                    gfx_Rectangle(10, ui_textstart_y + 20, 11, 11);
                    if(s->indexSplit) {
                        gfx_SetTextXY(12, ui_textstart_y + 22);
                        gfx_PrintChar('X');
                    }
                    
                    gfx_PrintStringXY("Enable Split Indexing Mode", 30, ui_textstart_y + 20);
                    gfx_PrintStringXY("Reserve checksum and size calculations", 40, ui_textstart_y + 30);
                    gfx_PrintStringXY("for first viewing in Program Options.", 40, ui_textstart_y + 40);
                    gfx_PrintStringXY("Less load time, slight scroll lag.", 40, ui_textstart_y + 50);
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
               (!strncmp(var_name, OSProp, 8)) ||
               (!strncmp(var_name, "AVsh", 4))){
                    ti_var_t fp = ti_OpenVar(var_name, "r", type);
                    if(fp) ti_SetArchiveStatus(true, fp);
                    ti_Close(fp);
                }
        }
    }
}

                  




int num_len(int num){
    int count = 0;
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


