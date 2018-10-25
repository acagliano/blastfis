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

#include "typedefs.h"
#include "files.h"
#include "asmstuff.h"
#include "gfx/all_gfx.h"

/* Put your function prototypes here */

char pgrm_MainMenu(char selected);
char pgrm_IntegMenu(unsigned char selected, unsigned char max);
void pgrm_EraseContent(void);
void pgrm_DrawBackground(void);
void pgrm_DrawSplashScreen(char selected);
void gfx_custom_wrappedtext(char *text, int x, int width);
bool time_IsFileOutdated(time_struct_short_t *file, uint8_t maxAge);

void av_About(void);
void av_Integrity(program_t* proglist, ti_var_t attributes, int selected, int max, gfx_sprite_t* sprites[]);


int integ_IsFileTracked(char *progname, ti_var_t attributes);
unsigned long av_CheckSumFile(char* progname, unsigned char type);

/* Put all your globals here. */
const char *ProgName = "Blast";
const char *SubName = "Calculator Security Suite";
const char *Version = "0.91b";

program_t proglist[256] = {0};
gfx_sprite_t* sprites[1];

time_struct_t systemtime;
settings_save_t settingsSave = {0};

void main(void) {
	/* Fill in the body of the main function here */
    char option = 0, prior_option = 0, i;
    char proglist_selected = 0;
    char* progname;
    ti_var_t f_attributes;
    char searchtypes[2] = {TI_PRGM_TYPE, TI_PPRGM_TYPE};
    int prognum = 0;
    sprites[0] = gfx_MallocSprite(tracking_icon_width, tracking_icon_height);
    zx7_Decompress(sprites[0], tracking_icon_compressed);
    ti_CloseAll();
    if((f_attributes = ti_Open(AttrDB, "r+")))
        ;
    else if(f_attributes = ti_Open(AttrDB, "w+"))
        ;
    else return;
    ti_Rewind(f_attributes);
    for(i = 0; i < sizeof(searchtypes); i++){
        uint8_t *search_pos = NULL;
        while((progname = ti_DetectVar(&search_pos, NULL, searchtypes[i])) != NULL) {
            if (strcmp(progname, "#") && strcmp(progname, "!")){
                proglist[prognum].progtype = searchtypes[i];
                strcpy(proglist[prognum++].progname, progname);
            }
        }
    }
    asm_RunIndicOff();
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_SetTextTransparentColor(33);
    gfx_SetTransparentColor(255);
    gfx_SetTextBGColor(33);
    gfx_SetClipRegion(0, 0, 320, 240);
    int_Disable();
    pgrm_DrawBackground();
    while(option != 6){
        unsigned char key = os_GetCSC();;
        if((key == sk_Up) && (option > 0)) option--;
        if((key == sk_Down) && (option < 5)) option++;
        if(key == sk_Clear) break;
        if(option == 1){
            if(key == sk_Stat) prior_option = 0;
            if((key == sk_Add) && (proglist_selected < (prognum-1))) {proglist_selected++; prior_option = 0;}
            if((key == sk_Sub) && (proglist_selected > 0)){proglist_selected--; prior_option = 0;}
            if(key == sk_Mode) {
                if((integ_IsFileTracked(proglist[proglist_selected].progname, f_attributes)) == -1){
                    ProgData_t source = {0};
                    strncpy(source.name, proglist[proglist_selected].progname, strlen(proglist[proglist_selected].progname));
                    source.type = proglist[proglist_selected].progtype;
                    source.checksum = av_CheckSumFile(proglist[proglist_selected].progname, source.type);
                    ti_Seek(0, SEEK_END, f_attributes);
                    ti_Write(&source, sizeof(ProgData_t), 1, f_attributes);
                    ti_Rewind(f_attributes);
                    prior_option = 0;
                }
            }
        }
        pgrm_DrawSplashScreen(option);
        switch(option){
            case 0:
                if(option != prior_option) pgrm_EraseContent();
                av_About();
                break;
            case 1:
                if(option != prior_option){
                    pgrm_EraseContent();
                    av_Integrity(&proglist, f_attributes, proglist_selected, prognum, sprites);
                }
                break;
        }
        prior_option = option;
    }
    ti_CloseAll();
    gfx_End();
	prgm_CleanUp();
    int_Enable();
    asm_ClrLCDFull();
    asm_DrawStatusBar();
    return;
}

/* Put other functions here */

void av_About(void){
    int i;
    int x = 115, y = 75, xmax = 310;
    static const char *about = "Blast2 is a file integrity|and malware detection|software. It can detect|changes to programs on your|calc and scan them for harm-|ful things. Optionally this|program can quarantine|harmful programs and|take snapshots (backups)|of programs, allowing you|to revert changes to|programs at any time.||For more information,|visit:|http://clrhome.org/blastav";
    gfx_SetTextFGColor(0);
    for(i = 0; i < strlen(about); i++) {
        if(about[i] == '|') {y += 9; x = 115;}
        else{
            gfx_SetTextXY(x, y);
            gfx_PrintChar(about[i]);
            x+=7;
        }
    }
    gfx_BlitBuffer();
}

void av_Integrity(program_t* proglist, ti_var_t attributes, int selected, int max, gfx_sprite_t* sprites[]){
    char i = 0;
    int index;
    int textx = 140, texty = 75;
    while( i < max ){
        if(selected == i){
            gfx_SetColor(139);
            gfx_FillRectangle(textx-25, texty-2, 318 - textx + 25, 11);
        }
        gfx_PrintStringXY(proglist[i].progname, textx, texty);
        //gfx_TransparentSprite(tracking_uncompressed, textx-14, texty-1);
        if((index = integ_IsFileTracked(proglist[i].progname, attributes)) != -1)
            gfx_TransparentSprite(sprites[0], textx-14, texty-1);
        texty += 10;
        i++;
    }
    gfx_BlitBuffer();
}
    

int integ_IsFileTracked(char *progname, ti_var_t attributes){
    int response = -1, i = 0;
    int test;
    ProgData_t check;
    while(ti_Read(&check, sizeof(ProgData_t), 1, attributes)){
        if(!strncmp(progname, check.name, 8)) {response = i; break;}
        i++;
    }
    return response;
}
       
unsigned long av_CheckSumFile(char* progname, unsigned char type){
    ti_var_t temp = ti_OpenVar(progname, "r", type);
    uint16_t size = ti_GetSize(temp);
    uint16_t i;
    unsigned long checksum = 0;
    for(i = 0; i < size; i++)
        checksum += ti_GetC(temp);
    ti_Close(temp);
    return checksum;
}
/*
void av_CheckSumAll(void){
    time_struct_short_t modified;
    uint16_t ypos = 0;
    
    // when called, destroys the pre-existing checksums database
    // recreates it and saves new values
    // for each installed program variable, we save:
    //      up to 9 bytes for name + type
    //      24-bit (3-byte) checksum
    //      date of last checksum also written
    boot_GetDate(&modified.day, &modified.month, &modified.year);
    ti_Write(&modified, sizeof(time_struct_short_t), 1, avData);
    gfx_ZeroScreen();
    gfx_PrintStringXY("Checksumming Programs...", 10, ypos);
    gfx_SetColor(135);
    gfx_HorizLine(0, 10, 320);
    gfx_SetColor(0);
    ypos += 13;
    
   
            // repeat until ti_DetectVar returns NULL
            // ti_DetectVar returns program name
                ProgData_t *program = malloc(sizeof(ProgData_t));      // init zero'd program data structure
                ti_var_t progdata = ti_OpenVar(progname, "r", TI_PRGM_TYPE);    // open var slot for program
                gfx_PrintStringXY(progname, 5, ypos);
                memset(program, '\0', sizeof(ProgData_t));
                program->type = TI_PRGM_TYPE;
                program->size = ti_GetSize(progdata);
                strcpy(program->name, progname);     // copy progname to struct
                while((tempread = ti_GetC(progdata)) != EOF){       // read out every byte of program
                    program->checksum += tempread;          // add data at tempread to checksum
                }
                if(ti_Write(program, sizeof(ProgData_t), 1, avData) == 1) gfx_PrintStringXY("success", 150, ypos);
                else gfx_PrintStringXY("failed", 150, ypos);
                ti_Close(progdata);
                free(program);
                ypos += 12;
                if(ypos > 227){
                    while(!os_GetCSC());
                    gfx_SetColor(0);
                    gfx_FillRectangle(0, 13, 320, 227);
                    ypos = 13;
                }
            }
        }
    }
    ti_SetArchiveStatus(true, avData);
    ti_Close(avData);
    free(progname);
    while(!os_GetCSC());
}




char av_ScanAll(void){
    char *progname, key, i;
    ti_var_t avDefs;
    int opstrsize, descsize;
    ti_CloseAll();
    if( !(avDefs = ti_Open(VDefs, "r")) ) return 1;
    if(ti_Seek(4, SEEK_SET, avDefs) == EOF) return 2;
    gfx_SetTextConfig(gfx_text_clip);
    while((opstrsize = ti_GetC(avDefs)) != EOF){
        int xpos = 0, ypos = 0;
        char searchtypes[2] = {TI_PRGM_TYPE, TI_PPRGM_TYPE}, i;
        opitem_t optemp = {0};
        gfx_ZeroScreen();
        gfx_PrintStringXY("Press [Clear] to abort at any time.",0, 228);
        // this should loop until EOF reached
        // tempread should have size of current byte sequence to scan for, or EOF
        ti_Read(&optemp.opcodestr, opstrsize, 1, avDefs);
        gfx_PrintStringXY(optemp.opcodestr, xpos, ypos);
        optemp.opcodesize = ti_GetC(avDefs);
        ti_Read(&optemp.opcodehex, optemp.opcodesize, 1, avDefs);
        descsize = ti_GetC(avDefs);
      //  ti_Read(&optemp.desc, descsize, 1, avDefs);
        gfx_PrintString("h");
     //   gfx_PrintStringXY(optemp.desc, xpos, ypos+12);
        gfx_SetColor(135);
        gfx_HorizLine(0, 24, 320);
        gfx_SetColor(0);
        ypos += 18;
        // Print on screen what byte sequence we are scanning for
        for(i = 0; i < sizeof(searchtypes); i++){
            uint8_t *search_pos = NULL;
            int tempread = 0;
            while((progname = ti_DetectVar(&search_pos, NULL, searchtypes[i])) != NULL) {
                if (strcmp(progname, "#") && strcmp(progname, "!") && strcmp(progname, "BLASTCSS")){
                    char *searchloc = &optemp.opcodehex;
                    uint8_t hits = 0;
                    ti_var_t tempfile = ti_OpenVar(progname, "r", searchtypes[i]);
                    while((tempread = ti_GetC(tempfile)) != EOF){
                        if((key = os_GetCSC()) == sk_Clear) {
                            ti_CloseAll();
                            break;
                        }
                        if((char)tempread == *searchloc){
                            hits++;
                            if(hits == optemp.opcodesize){
                                uint16_t position = ti_Tell(tempfile) - optemp.opcodesize;
                                char hextemp[7];
                                ypos += 12;
                                gfx_PrintStringXY(progname, xpos, ypos);
                                gfx_PrintStringXY("offset: ", xpos+90, ypos);
                                gfx_PrintUInt(position, 1 + (position > 9) + (position > 99) + (position > 999) + (position > 9999));
                                gfx_PrintString(", addr: 0x");
                                sprintf(hextemp, "%06X", ti_GetDataPtr(tempfile));
                                gfx_PrintString(hextemp);
                                gfx_PrintString("h");
                                break;
                            }
                            searchloc++;
                        } else {
                            searchloc = &optemp.opcodehex;
                            hits = 0;
                        }
                    }
                    if(key == sk_Clear) break;
                    // repeat until ti_DetectVar returns NULL
                    // ti_DetectVar returns program name
                    // should simply output any filename containing byte sequence
                    ti_Close(tempfile);
                }
            }
            if(key == sk_Clear) break;
        }
        if(key == sk_Clear) break;
        ypos += 10;
        gfx_PrintStringXY("Search complete. Any key to proceed...", xpos, ypos);
        while(!os_GetCSC());    // wait for keypress, hopefully give user time to write down
    }
    ti_SetArchiveStatus(true, avDefs);
    ti_Close(avDefs);
    gfx_SetTextConfig(gfx_text_noclip);
    return 0;
}


char av_ValidateSaved(void){
   
}
*/
void pgrm_EraseContent(void){
    gfx_SetColor(205);
    gfx_FillRectangle(111, 71, 319-111, 219-71);
}

void pgrm_DrawBackground(void){
    // draw background
    gfx_FillScreen(205);
    gfx_SetColor(0);
    gfx_FillRectangle(0, 0, 320, 70);
    gfx_SetColor(40);
    gfx_FillRectangle(4, 4, 320-8, 70-8);
    gfx_SetTextFGColor(255);
    gfx_SetTextScale(3,3);
    gfx_PrintStringXY("B L A S T", 100, 20);
    gfx_SetTextScale(2,2);
    gfx_PrintStringXY("2", 265, 15);
    gfx_SetTextScale(1,1);
    gfx_PrintStringXY("TI File Integrity Software", 100, 50);
    gfx_SetTextFGColor(0);
    gfx_PrintStringXY("(c) 2018 - Anthony Cagliano, ClrHome", 5, 228);
    gfx_BlitBuffer();
}

void pgrm_DrawSplashScreen(char selected) {
    // Draw Menu w item selected
    char i;
    gfx_SetTextFGColor(0);
    for(i = 0; i < 6; i++){
        char *label;
        gfx_SetColor(0);
        gfx_FillRectangle(0, i * 25 + 70, 110, 25);
        if(selected == i) gfx_SetColor(139);
        else gfx_SetColor(172);
        gfx_FillRectangle(2, i * 25 + 72, 106, 21);
        switch(i){
            case 0:
                label = "About Blast2";
                break;
            case 1:
                label = "File Checking";
                break;
            case 2:
                label = "Quarantine";
                break;
            case 3:
                label = "Snapshots";
                break;
            case 4:
                label = "Settings";
                break;
            case 5:
                label = "Program Help";
                break;
            default:
                label = "Unimplemented";
                break;
                
        }
        gfx_PrintStringXY(label, 5, i * 25 + 79);
    }
    gfx_BlitBuffer();
}


// currently unused but might be needed
bool time_IsFileOutdated(time_struct_short_t *file, uint8_t maxAge){
    uint24_t systemdays, filedays;
    systemdays = (systemtime.year * 365) + (systemtime.month * 30) + systemtime.day;
    filedays = (file->year * 365) + (file->month * 30) + file->day;
    if(systemdays - filedays >= maxAge) return 1;
    return 0;
    
}


