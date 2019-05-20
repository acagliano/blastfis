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

/* Put your function prototypes here */

void pgrm_EraseContent(void);
void pgrm_DrawBackground(gfx_sprite_t *icon);


int text_GetCenterX(char* string, int width);
int num_len(int num);
int progsort(const void* a, const void* b);
void av_ScanFile(progname_t* program);
void av_SendAllToArchive(void);

/* Put all your globals here. */
const char *ProgName = "Blast";
const char *SubName = "Calculator Security Suite";
const char *Version = "0.91b";

/* Supporting Files */

#define OS_START 0x02000h
#define CERT_START 0x3B0000h
#define ARCH_START 0x0C0000h
#define RAM_START 0xD00000h
#define ui_textstart_y 75
#define ui_progdata_out 175

const char strings[][14] = {"File Options", "System Scans", "Snapshots", "Settings", "About", "Quit"};
const char desc[][60] = {"View and modify file data.",
    "Scan all RAM, Archive, or OS sectors.",
    "Change how this program works.",
    "View information about this program.",
    "Exit the program."};
progopt_t progopt_yvals[] = {
    {0, 0},
    {106, ui_textstart_y + 75},
    {106, ui_textstart_y + 85},
    {106, ui_textstart_y + 95},
    {106, ui_textstart_y + 120}
};

void main(void) {
    bool progRun = true, firstLoop = true;
    char i;
    char screen = MAIN;
    uint8_t *search_pos;
    progname_t* prognames = NULL;
    ti_var_t propfile;
    int num_programs = av_GetNumFiles();
    int num_snaps;
    gfx_sprite_t* logo = gfx_MallocSprite(blast_icon_width, blast_icon_height);
    gfx_sprite_t* integ_pass = gfx_MallocSprite(integ_pass_icon_width, integ_pass_icon_height);
    gfx_sprite_t* integ_fail = gfx_MallocSprite(integ_fail_icon_width, integ_fail_icon_height);
    selected_t selected = {0};
    //allocate memory
   // int_Disable();
    zx7_Decompress(logo, blast_icon_compressed);
    zx7_Decompress(integ_pass, integ_pass_icon_compressed);
    zx7_Decompress(integ_fail, integ_fail_icon_compressed);
    ti_CloseAll();
    if(!(propfile = ti_Open(PropDB, "r+")))
        propfile = ti_Open(PropDB, "w+");
    ti_Close(propfile);
    if(!(propfile = ti_Open(SnapDB, "r+")))
        propfile = ti_Open(SnapDB, "w+");
    num_snaps = (ti_GetSize(propfile) / sizeof(snapshot_t));
    ti_Close(propfile);
    gfx_Begin();
    gfx_SetTextTransparentColor(1);
    gfx_SetTextBGColor(1);
    gfx_SetDrawBuffer();
    // loop save names of all files on device
    prognames = (progname_t*)calloc(num_programs, sizeof(progname_t));
    if(prognames == NULL) exit(1);
    av_GenerateFileIndex(prognames, num_programs);
    // decompress all graphics
    do {
        unsigned char key = os_GetCSC();
        char i;
        char progheap, heapoffset;
        bool refresh = firstLoop;
        progsave_t* current;
        snapshot_t* snap;
        unsigned long checksum = 0;
        char cs_string[11] = {'\0'};
        progname_t* prog;
        if(propfile = ti_Open(SnapDB, "r+")){
            num_snaps = (ti_GetSize(propfile) / sizeof(snapshot_t));
            ti_Close(propfile);
        }
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
                    if(selected.snapopt)
                        if(selected.snapopt < 2) selected.snapopt++;
                    break;
             
            }
        }
        if(key == sk_Right){
            switch(screen){
                case SNAPSHOTS:
                    if(selected.snapshot < (num_snaps-1)) selected.snapshot++;
                    break;
            }
        }
        if(key == sk_Left){
            switch(screen){
                case SNAPSHOTS:
                    if(selected.snapshot > 0) selected.snapshot--;
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
                    if(selected.snapopt > 1) selected.snapopt--;
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
            if(propfile = ti_Open(SnapDB, "r+")){
                num_snaps = (ti_GetSize(propfile) / sizeof(snapshot_t));
                ti_Close(propfile);
            }
        }
       /* if(key == sk_Left){
            switch(screen){
               
            }
        }
        if(key == sk_Right){
            switch(screen){
                
            }
        } */
        
        if(key == sk_Enter) {
            if(screen == MAIN) screen = selected.menu;
            else if(screen == FILE_OPTS){
                if(!selected.progopt) selected.progopt = 1;
                else if(selected.progopt == TRACK_TOGG)
                    av_TogglePropTrack(&prognames[selected.program]);
                else if(selected.progopt == TRACK_UPD)
                    av_UpdateAttributes(&prognames[selected.program]);
                else if(selected.progopt == SCAN)
                    av_ScanFile(&prognames[selected.program]);
                else if(selected.progopt == SNAP_ENABLE){
                    if(av_SnapDB_GetSlotMatchOffset(&prognames[selected.program]) == -1)
                        av_CreateSnapshot(&prognames[selected.program]);
                    else
                        av_UpdateSnapshot(&prognames[selected.program], selected.snapshot);
                        
                }
            }
            else if(screen == SNAPSHOTS){
                if(!selected.snapopt) selected.snapopt = 1;
                else {
                    if(selected.snapopt == 1) av_RestoreSnapshot(selected.snapshot);
                    if(selected.snapopt == 2) av_DeleteSnapshot(selected.snapshot, num_snaps);
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
                if(selected.snapopt) selected.snapopt = 0;
                else screen = MAIN;
            }
            else screen = MAIN;
        }
        if(key) refresh = true;
        if(refresh) {
            pgrm_DrawBackground(logo);
            switch(screen){
                case MAIN:
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
                case FILE_OPTS:
                    pgrm_EraseContent();
                    gfx_SetTextScale(2,2);
                    gfx_PrintStringXY("FILE OPTIONS", 5, 75);
                    gfx_SetTextScale(1,1);
                    progheap = selected.program / 13 * 13;
                    for(i = progheap; i < (progheap + 14); i++){
                        heapoffset = i - progheap;
                        gfx_SetTextFGColor(0);
                        if(i < num_programs){
                            prog = &prognames[i];
                            if(i == selected.program){
                                gfx_SetColor(40); gfx_SetTextFGColor(255);
                                gfx_FillRectangle(3, heapoffset * 10 + ui_textstart_y + 18, 70, 11);
                            }
                            gfx_PrintStringXY(&prog->name[0], 5, heapoffset * 10 + ui_textstart_y + 20);
                        }
                    }
                    gfx_SetTextFGColor(0);
                    prog = &prognames[selected.program];
                    if(selected.progopt){
                        int x = progopt_yvals[selected.progopt].x;
                        int y = progopt_yvals[selected.progopt].y - 1;
                        gfx_SetColor(140);
                        gfx_FillRectangle(x, y, 150, 10);
                    }
                    gfx_PrintStringXY("File Name: ", 100, ui_textstart_y + 20);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    gfx_PrintString(&prog->name[0]/*, 10, 9 * i + 75*/);
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
                     if(snap = av_SnapDB_IndexToPtr(av_SnapDB_GetSlotMatchOffset(prog))){
                         char timestring[11];
                         dbg_sprintf(dbgout, "Snap Looking at: %u", snap);
                         sprintf(timestring, "%02u:%02u:%04u", snap->time.month, snap->time.day, snap->time.year);
                         gfx_PrintString(timestring);
                         gfx_PrintStringXY("Update ", 110, ui_textstart_y + 120);
                    }
                     else {
                         gfx_PrintString("none");
                         gfx_PrintStringXY("Create ", 110, ui_textstart_y + 120);
                     }
                    gfx_PrintString("Snapshot");
                    break;
                case SNAPSHOTS:
                {
                    size_t size;
                    snapshot_t* snap = av_SnapDB_IndexToPtr(selected.snapshot);
                    char timestring[11];
                    pgrm_EraseContent();
                    gfx_SetTextScale(2,2);
                    gfx_PrintStringXY("SNAPSHOTS", 5, 75);
                    gfx_SetTextScale(1,1);
                    gfx_PrintStringXY("File Name: ", 100, ui_textstart_y + 40);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    gfx_PrintString(snap->fname/*, 10, 9 * i + 75*/);
                    gfx_PrintStringXY("File Type: ", 100, ui_textstart_y + 50);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    if(snap->type == TI_PRGM_TYPE || snap->type == TI_PPRGM_TYPE)
                        gfx_PrintString("Program");
                    else gfx_PrintString("AppVar");
                    gfx_PrintStringXY("File Size: ", 100, ui_textstart_y + 60);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    gfx_PrintUInt(snap->size, num_len(snap->size));
                    gfx_PrintStringXY("Modified: ", 100, ui_textstart_y + 70);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    sprintf(timestring, "%02u:%02u:%04u", snap->time.month, snap->time.day, snap->time.year);
                    gfx_PrintString(timestring);
                    if(selected.snapopt){
                        gfx_SetColor(140);
                        gfx_FillRectangle(105, ui_textstart_y + 74 + (selected.snapopt * 10) , 150, 10);
                    }
                    gfx_PrintStringXY("Restore Snapshot", 110, ui_textstart_y + 85);
                    gfx_PrintStringXY("Delete Snapshot", 110, ui_textstart_y + 95);
                    break;
                }
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
    gfx_End();
    av_SendAllToArchive();
    pgrm_CleanUp();
}

void av_SendAllToArchive(void){
    char *var_name;
    uint8_t *search_pos = NULL;
    uint8_t type;
    while((var_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL) {
        if (type == TI_PRGM_TYPE || type == TI_PPRGM_TYPE || type == TI_APPVAR_TYPE){
            if((!strncmp(var_name, SnapDB, 8)) ||
               (!strncmp(var_name, PropDB, 8)) ||
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



void av_ScanFile(progname_t* program){
    
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


