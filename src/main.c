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

/* Other available headers */
// stdarg.h, setjmp.h, assert.h, ctype.h, float.h, iso646.h, limits.h, errno.h, debug.h
#define FORCE_INTERRUPTS
#include <graphx.h>
#include <fileioc.h>
#include <compression.h>
#include <intce.h>

#include "attributes.h"
#include "timestamps.h"
#include "gfx/all_gfx.h"
#include "crypto.h"
#include "indexing.h"
#include "menuopts.h"

/* Put your function prototypes here */

void pgrm_EraseContent(void);
void pgrm_DrawBackground(gfx_sprite_t *icon);


int text_GetCenterX(char* string, int width);
int num_len(int num);
int progsort(const void* a, const void* b);
void av_TogglePropTrack(progname_t* program);
void enable_PropTrack(progname_t* program);
void disable_PropTrack(progname_t* program);
void av_ToggleVersControl(progname_t* program);
void av_ScanFile(progname_t* program);

/* Put all your globals here. */
const char *ProgName = "Blast";
const char *SubName = "Calculator Security Suite";
const char *Version = "0.91b";

/* Supporting Files */
const char *PropDB = "AVPropDB";
const char *AvDB = "AVDefsDB";

const char strings[][14] = {"File Options", "System Scans", "Settings", "About", "Quit"};
const char desc[][60] = {"View and modify file data.",
    "Scan all RAM, Archive, or OS sectors.",
    "Change how this program works.",
    "View information about this program.",
    "Exit the program."};



#define OS_START 0x02000h
#define CERT_START 0x3B0000h
#define ARCH_START 0x0C0000h
#define RAM_START 0xD00000h
#define ui_textstart_y 75
#define ui_progdata_out 175

void main(void) {
    bool progRun = true, firstLoop = true;
    char i;
    char screen = MAIN;
    uint8_t *search_pos;
    progname_t* prognames = NULL;
    int num_program = av_GetNumFiles();
    gfx_sprite_t* logo = gfx_MallocSprite(blast_icon_width, blast_icon_height);
    gfx_sprite_t* integ_pass = gfx_MallocSprite(integ_pass_icon_width, integ_pass_icon_height);
    gfx_sprite_t* integ_fail = gfx_MallocSprite(integ_fail_icon_width, integ_fail_icon_height);
    selected_t selected = {0};
    //allocate memory
   // int_Disable();
    zx7_Decompress(logo, blast_icon_compressed);
    zx7_Decompress(integ_pass, integ_pass_icon_compressed);
    zx7_Decompress(integ_fail, integ_pass_icon_compressed);
    ti_CloseAll();
    if(!(propfile = ti_Open(PropDB, "r+")))
        propfile = ti_Open(PropDB, "w+");
    ti_Close(propfile);
    gfx_Begin();
    gfx_SetTextTransparentColor(1);
    gfx_SetTextBGColor(1);
    gfx_SetDrawBuffer();
    gfx_PrintStringXY("Indexing device contents...", 5, 5); gfx_BlitBuffer();
    // loop save names of all files on device
    prognames = (progname_t*)malloc(num_programs * sizeof(progname_t));
    memset(prognames, 0, sizeof(progname_t) * num_programs);
    av_GenerateFileIndex(prognames);
  
    // decompress all graphics
    do {
        unsigned char key = os_GetCSC();
        char i;
        char progheap, heapoffset;
        bool refresh = firstLoop;
        unsigned long checksum = 0;
        char cs_string[11] = {'\0'};
        progname_t* prog;
        if(key == sk_Down){
            switch(screen){
                case MAIN:
                    if(selected.menu < QUIT) selected.menu++;
                    break;
                case FILE_OPTS:
                    if(!selected.progopt) {
                        if(selected.program < (num_programs-1)) selected.program++;
                    }else{
                        if(selected.progopt < SCAN) selected.progopt++;}
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
                else if(selected.progopt == 1)
                    av_TogglePropTrack(&prognames[selected.program]);
                else if(selected.progopt == 2)
                    av_ToggleVersControl(&prognames[selected.program]);
                else if(selected.progopt == 3)
                    av_ScanFile(&prognames[selected.program]);
            }
        }
        if(key == sk_Clear){
            if(screen == MAIN) progRun = false;
            if(screen == FILE_OPTS){
                if(selected.progopt) selected.progopt = 0;
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
                    gfx_SetColor(40); gfx_FillRectangle(90, selected.menu * 25 + 74, 130, 22);
                    for(i = 0; i < NUM_SCREENS; i++){
                        gfx_SetColor(0);
                        gfx_FillRectangle(100, i * 25 + ui_textstart_y, 110, 20);
                        gfx_SetColor(172);
                        gfx_FillRectangle(102, i * 25 + ui_textstart_y + 2, 106, 16);
                        gfx_PrintStringXY(strings[i], 102 + text_GetCenterX(strings[i], 106), i * 25 + ui_textstart_y + 6);
                    }
                    gfx_PrintStringXY(desc[selected.menu], text_GetCenterX(desc[selected.menu], LCD_WIDTH), 205);
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
                    gfx_PrintStringXY("File: ", 100, ui_textstart_y + 20);
                    gfx_PrintUInt(selected.program + 1, num_len(selected.program + 1));
                    gfx_PrintString(" / ");
                    gfx_PrintUInt(num_programs, num_len(num_programs));
                    gfx_PrintStringXY("File Name: ", 100, ui_textstart_y + 35);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    gfx_PrintString(&prog->name[0]/*, 10, 9 * i + 75*/);
                    gfx_PrintStringXY("File Type: ", 100, ui_textstart_y + 45);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    if(prog->type == TI_PRGM_TYPE || prog->type == TI_PPRGM_TYPE)
                        gfx_PrintString("Program");
                    else gfx_PrintString("AppVar");
                    gfx_PrintStringXY("File Size: ", 100, ui_textstart_y + 60);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    gfx_PrintUInt(prog->size, num_len(prog->size));
                    gfx_PrintStringXY("CRC-32 CS: ", 100, ui_textstart_y + 70);
                    gfx_SetTextXY(ui_progdata_out, gfx_GetTextY());
                    sprintf(cs_string, "%xh", prog->checksum);
                    gfx_PrintString(cs_string);
                    gfx_PrintStringXY("Attr Tracking: ", 100, ui_textstart_y + 80);
                    gfx_SetTextXY(ui_progdata_out + 30, gfx_GetTextY());
                    if(prog->prop_track) {
                        ti_var_t propdb = ti_Open(PropDB, "r+");
                        progsave_t* save = prog->prop_track + ti_GetDataPtr(propdb);
                        ti_Close(propdb);
                        gfx_PrintString("enabled");
                        if(save->checksum == prog->checksum)
                            gfx_TransparentSprite(integ_pass, ui_progdata_out + 85, ui_textstart_y + 78);
                        else
                            gfx_TransparentSprite(integ_fail, ui_progdata_out + 85, ui_textstart_y + 78);
                    }
                    else gfx_PrintString("disabled");
                    gfx_PrintStringXY("Vers Tracking: ", 100, ui_textstart_y + 90);
                    gfx_SetTextXY(ui_progdata_out + 30, gfx_GetTextY());
                    if(prog->vers_track) gfx_PrintString("enabled");
                    else gfx_PrintString("disabled");
                    if(selected.progopt){
                        gfx_SetColor(140);
                        gfx_FillRectangle(116, (selected.progopt - 1) * 10 + ui_textstart_y + 99, 160, 10);
                    }
                    gfx_PrintStringXY("Toggle Attr Tracking", 120, ui_textstart_y + 100);
                    gfx_PrintStringXY("Toggle Vers Tracking", 120, ui_textstart_y + 110);
                    gfx_PrintStringXY("Verify Attributes", 120, ui_textstart_y + 120);
                    gfx_PrintStringXY("Scan File", 120, ui_textstart_y + 130);
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
    gfx_End();
    pgrm_CleanUp();
}


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
    ti_var_t filein, fileout;               // open two file streams, one to read, one to write
    progsave_t read;                        // create temp intermediary copy of save
    bool found = false;
    if((filein = ti_Open(PropDB, "r+")) && (fileout = ti_Open(PropDB, "r+"))){  // open two file streams
        while(ti_Read(&read, sizeof(progsave_t), 1, fileout) == 1){     // while there's data to read
            if((!memcmp(program->name, read.name, 8)) && (read.type == program->type)){    // if name found and type match
                program->prop_track = 0;            // zero prop_track save (remove offset)
                found = true;
                break;                              // break with file read stream pointing to after match
            }
        }
        if(found){
            ti_Seek(ti_Tell(fileout) - sizeof(progsave_t), SEEK_SET, filein);   // set file write stream to read offset - sizeof a progsave
            while(ti_Read(&read, sizeof(progsave_t), 1, fileout) == 1)  // read from read stream
                ti_Write(&read, sizeof(progsave_t), 1, filein);     // write to write stream
        }       // read stream should always be one block ahead of write stream
        ti_Close(fileout);      // close read stream
        ti_Resize(ti_GetSize(filein) - sizeof(progsave_t), filein); // decrease file size by 1 progsave block
        ti_Close(filein);       // close write stream
    }
}



void av_ToggleVersControl(progname_t* program){
    
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


