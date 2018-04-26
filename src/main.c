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
#include <decompress.h>
#include <intce.h>

#include "typedefs.h"
#include "files.h"
#include "asmstuff.h"
#include "gfx/all_gfx.h"

/* Put your function prototypes here */
void av_CheckSumAll(void);
char av_ScanAll(void);
char av_ValidateSaved(void);
void showBoxes(void);
char pgrm_DrawMainMenu(char selected);
void pgrm_DrawSettingsMenu(void);
void pgrm_DrawSplashScreen(void);
void pgrm_ApplySettings(void);
void pgrm_SaveSettings(void);
bool time_IsFileOutdated(time_struct_short_t *file, uint8_t maxAge);

/* Put all your globals here. */
const char *ProgName = "Blast";
const char *SubName = "Calculator Security Suite";
const char *Version = "0.8b";

time_struct_t systemtime;
settings_save_t settingsSave = {0};

void main(void) {
	/* Fill in the body of the main function here */
    char option;
    ti_var_t settings;
    time_struct_t *savedtime = &settingsSave.time;
    asm_RunIndicOff();
    gfx_Begin();
    gfx_SetClipRegion(0, 0, 320, 240);
    ti_CloseAll();
    if(settings = ti_Open(AVSettings, "r")){
        ti_Read(&settingsSave, sizeof(settings_save_t), 1, settings);
        ti_Close(settings);
    }
        // check and restore or save date
    boot_GetDate(&systemtime.day, &systemtime.month, &systemtime.year);
    boot_GetTime(&systemtime.sec, &systemtime.min, &systemtime.hour);
    if( systemtime.year < savedtime->year ) {
        boot_SetDate(savedtime->day, savedtime->month, savedtime->year);
        boot_SetTime(savedtime->sec, savedtime->min, savedtime->hour);
    } else {
        savedtime->day = systemtime.day;
        savedtime->month = systemtime.month;
        savedtime->year = systemtime.year;
        savedtime->sec = systemtime.sec;
        savedtime->min = systemtime.min;
        savedtime->hour = systemtime.hour;
    }
    pgrm_ApplySettings();
    ti_CloseAll();
    int_Disable();
    pgrm_DrawSplashScreen();
    option = 1;
    while((option = pgrm_DrawMainMenu(option)) != 6){
        switch(option){
            case 1:
                av_ScanAll();
                gfx_ZeroScreen();
                pgrm_DrawSplashScreen();
                // error conditions not implemented
                break;
            case 2:
                av_ValidateSaved();
                gfx_ZeroScreen();
                pgrm_DrawSplashScreen();
                break;
            case 3:
                av_CheckSumAll();
                gfx_ZeroScreen();
                pgrm_DrawSplashScreen();
                break;
            case 4:
                pgrm_DrawSettingsMenu();
                gfx_ZeroScreen();
                pgrm_DrawSplashScreen();
                break;
            case 5:
                gfx_SetTextFGColor(135);
                gfx_ZeroScreen();
                gfx_PrintStringXY("About Blast CSS", 50, 0);
                gfx_HorizLine(0, 10, 320);
                gfx_PrintStringXY("Calculator Security for the TI-84+ CE", 0, 13);
                gfx_PrintStringXY("- Retains checksums and sizes of all vars", 0, 25);
                gfx_PrintStringXY("- Can verify both against current values", 0, 37);
                gfx_PrintStringXY("- Scans all vars based on defs. file", 0, 49);
                gfx_PrintStringXY("- Smartly records/restores time/date", 0, 61);
                gfx_PrintStringXY("- Community-sourced malware defs.", 0, 73);
                gfx_PrintStringXY("More info @ http://clrhome.org/blastav", 0, 85);
                gfx_PrintStringXY("by Anthony Cagliano", 0, 107);
                gfx_PrintStringXY("Any key to return to main menu", 0, 227);
                while(!os_GetCSC());
                gfx_ZeroScreen();
                pgrm_DrawSplashScreen();
                break;
        }
    }
    int_Enable();
    ti_CloseAll();
    gfx_End();
	prgm_CleanUp();
    asm_ClrLCDFull();
    asm_DrawStatusBar();
}

/* Put other functions here */

void av_CheckSumAll(void){
    char *progname, tempread;
    time_struct_short_t modified;
    uint16_t ypos = 0;
    char searchtypes[2] = {TI_PRGM_TYPE, TI_PPRGM_TYPE}, i = 0;
    ti_var_t avData = ti_Open(ScanDB, "w");
    
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
    
    for(i = 0; i < sizeof(searchtypes); i++){
        uint8_t *search_pos = NULL;
        while((progname = ti_DetectVar(&search_pos, NULL, searchtypes[i])) != NULL) {
            // repeat until ti_DetectVar returns NULL
            // ti_DetectVar returns program name
            if (strcmp(progname, "#") && strcmp(progname, "!")){
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
    char *progname;
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
        // this should loop until EOF reached
        // tempread should have size of current byte sequence to scan for, or EOF
        ti_Read(&optemp.opcodestr, opstrsize, 1, avDefs);
        gfx_PrintStringXY(optemp.opcodestr, xpos, ypos);
        optemp.opcodesize = ti_GetC(avDefs);
        ti_Read(&optemp.opcodehex, optemp.opcodesize, 1, avDefs);
        descsize = ti_GetC(avDefs);
        ti_Read(&optemp.desc, descsize, 1, avDefs);
        gfx_PrintString("h");
        gfx_PrintStringXY(optemp.desc, xpos, ypos+12);
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
                        if((char)tempread == *searchloc){
                            hits++;
                            if(hits == optemp.opcodesize){
                                ypos += 12;
                                gfx_PrintStringXY(progname, xpos, ypos);
                                break;
                            }
                            searchloc++;
                        } else {
                            searchloc = &optemp.opcodehex;
                            hits = 0;
                        }
                    }
                    // repeat until ti_DetectVar returns NULL
                    // ti_DetectVar returns program name
                    // should simply output any filename containing byte sequence
                    ti_Close(tempfile);
                }
            }
        }
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
    ti_var_t avData;
    char tempread;
    uint16_t xpos = 10, ypos = 0;
    ti_CloseAll();
    if( !(avData = ti_Open(ScanDB, "r")) ) return 1;
    // when called, destroys the pre-existing checksums database
    // recreates it and saves new values
    // for each installed program variable, we save:
    //      up to 9 bytes for name + type
    //      24-bit (3-byte) checksum
    //      date of last checksum also written
    if(ti_Seek(4, SEEK_SET, avData) == EOF) return 2;
    gfx_ZeroScreen();
    gfx_PrintStringXY("Verifying Checksums...", xpos, ypos);
    gfx_SetColor(135);
    gfx_HorizLine(0, 10, 320);
    gfx_SetColor(0);
    xpos = 5;
    ypos += 13;
    
    while(ti_Tell(avData) < ti_GetSize(avData)) {
        // repeat until ti_DetectVar returns NULL
        // ti_DetectVar returns program name
        ti_var_t progdata;
        uint24_t checksum = 0;
        ProgData_t *program = malloc(sizeof(ProgData_t));   // init zero'd program data structure
        gfx_SetTextFGColor(135);
        memset(program, '\0', sizeof(ProgData_t));
        ti_Read(program, sizeof(ProgData_t), 1, avData);
        
        if(progdata = ti_OpenVar(program->name, "r", program->type)){    // open var slot for program
            gfx_PrintStringXY(program->name, xpos, ypos);
            xpos += 150;
            if(program->size == ti_GetSize(progdata)) gfx_SetTextFGColor(135);
            else gfx_SetTextFGColor(192);
            gfx_PrintStringXY("size", xpos, ypos);
            xpos += 75;
            
            while((tempread = ti_GetC(progdata)) != EOF){       // read out every byte of program
                checksum += tempread;          // add data at tempread to checksum
            }
            if(program->checksum == checksum) gfx_SetTextFGColor(135);
            else gfx_SetTextFGColor(192);
            gfx_PrintStringXY("checksum", xpos, ypos);
            ti_Close(progdata);
            free(program);
            xpos = 5;
            ypos += 12;
            if(ypos > 227){
                while(!os_GetCSC());
                gfx_SetColor(0);
                gfx_FillRectangle(0, 13, 320, 227);
                ypos = 12;
            }
        }
    }
    while(!os_GetCSC());
    ti_Close(avData);
    return 0;
}

void showBoxes(void){
    gfx_sprite_t *uncompressed_off, *uncompressed_on;
    uncompressed_off = gfx_MallocSprite(8,8);
    dzx7_Standard(funcoff_compressed, uncompressed_off);
    uncompressed_on = gfx_MallocSprite(8,8);
    dzx7_Standard(funcon_compressed, uncompressed_on);
    if(settingsSave.hookInstall){ gfx_Sprite(uncompressed_on, 280, 112); }
    else { gfx_Sprite(uncompressed_off, 280, 112); }
    if(settingsSave.smartAttr){ gfx_Sprite(uncompressed_on, 280, 124); }
    else { gfx_Sprite(uncompressed_off, 280, 124); }
    if(settingsSave.enableFirewall){ gfx_Sprite(uncompressed_on, 280, 136); }
    else {gfx_Sprite(uncompressed_off, 280, 136);}
    if(settingsSave.enableQuarantine) {gfx_Sprite(uncompressed_on, 280, 148);}
    else {gfx_Sprite(uncompressed_off, 280, 148);}
    free(uncompressed_on);
    free(uncompressed_off);
}


char pgrm_DrawMainMenu(char selected){
    char key = 0;
    ti_CloseAll();
    gfx_PrintStringXY("BLASTCSS MAIN MENU", 130, 86);
    gfx_SetColor(135);
    gfx_HorizLine(130, 96, 135);
    gfx_SetColor(0);
    gfx_PrintStringXY("Scan Programs", 130, 100);
    gfx_PrintStringXY("Verify Attributes", 130, 112);
    gfx_PrintStringXY("Update Attributes File", 130, 124);
    gfx_PrintStringXY("Advanced Settings", 130, 136);
    gfx_PrintStringXY("About", 130, 148);
    gfx_PrintStringXY("Exit", 130, 160);
    gfx_SetColor(135);
    gfx_FillCircle(120, selected * 12 + 91, 3);
    while( (key = os_GetCSC()) != sk_Enter ){
        if(key){
            gfx_SetColor(0);
            gfx_FillCircle(120, selected * 12 + 91, 3);
            gfx_SetColor(135);
        }
        if(key == sk_Up){
            selected--;
            if(selected < 1) selected = 6;
        }
        if(key == sk_Down){
            selected++;
            if(selected > 6) selected = 1;
        }
        if(key) gfx_FillCircle(120, selected * 12 + 91, 3);
        if(key == sk_Clear) { selected = 6; break; }
    }
    return selected;
}


void pgrm_DrawSettingsMenu(void){
    char selected = 1, key = 0;
    ti_CloseAll();
    gfx_ZeroScreen();
    pgrm_DrawSplashScreen();
    gfx_PrintStringXY("BLASTCSS SETTINGS MENU", 130, 98);
    gfx_SetColor(135);
    gfx_HorizLine(130, 108, 166);
    gfx_SetColor(0);
    gfx_PrintStringXY("Enable Smart-Detect", 130, 112);
    gfx_PrintStringXY("Enable Smart-Attrib.", 130, 124);
    gfx_PrintStringXY("Enable Firewall", 130, 136);
    gfx_PrintStringXY("Enable Quarantine", 130, 148);
    gfx_PrintStringXY("Main Menu", 130, 160);
    showBoxes();
    gfx_SetColor(135);
    gfx_FillCircle(120, selected * 12 + 103, 3);
    gfx_SetColor(0);
    while( (key = os_GetCSC()) != sk_Clear ){
        if(key){
            gfx_SetColor(0);
            gfx_FillCircle(120, selected * 12 + 103, 3);
            gfx_SetColor(135);
        }
        if(key == sk_Up){
            selected--;
            if(selected < 1) selected = 5;
        }
        if(key == sk_Down){
            selected++;
            if(selected > 5) selected = 1;
        }
        if(key == sk_Enter){
            switch(selected){
                case 1:
                    settingsSave.hookInstall = !settingsSave.hookInstall;
                    showBoxes();
                    break;
                case 2:
                    settingsSave.smartAttr = !settingsSave.smartAttr;
                    showBoxes();
                    break;
                case 3:
                    settingsSave.enableFirewall = !settingsSave.enableFirewall;
                    showBoxes();
                    break;
                case 4:
                    settingsSave.enableQuarantine = !settingsSave.enableQuarantine;
                    showBoxes();
                    break;
                case 5:
                    key = sk_Clear;
                    break;
            }
        }
        if(key) gfx_FillCircle(120, selected * 12 + 103, 3);
        if(key == sk_Clear) break;
    }
    pgrm_SaveSettings();
    pgrm_ApplySettings();
}

void pgrm_DrawSplashScreen(void) {
    // Draw splash screen
    gfx_sprite_t *uncompressed, *scaled;
    ti_var_t avData;
    uint16_t temp;
    ti_CloseAll();
    gfx_ZeroScreen();
    gfx_SetTextFGColor(135);
    gfx_SetTextBGColor(0);
    gfx_SetTextScale(4, 4);
    gfx_PrintStringXY(ProgName, 100, 20);
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY(SubName, 100, 55);
    gfx_SetTextXY(5, 10);
    gfx_PrintString("v");
    gfx_PrintString(Version);
    uncompressed = gfx_MallocSprite(64,64);
    dzx7_Standard(blast_logo_compressed, uncompressed);
    scaled = gfx_MallocSprite(96, 96);
    scaled->width = 96;
    scaled->height = 96;
    gfx_ScaleSprite(uncompressed, scaled);
    gfx_Sprite(scaled, 10, 90);
    free(scaled);
    free(uncompressed);
    
    // Status Indicators
    gfx_PrintStringXY("Attributes File: ", 30, 212);
    if((avData = ti_Open(ScanDB, "r"))){
        time_struct_short_t modified;
        ti_Read(&modified, sizeof(time_struct_short_t), 1, avData);
        gfx_PrintUInt((int)modified.month, 2);
        gfx_PrintString("-");
        gfx_PrintUInt((int)modified.day, 2);
        gfx_PrintString("-");
        gfx_PrintUInt(modified.year, 4);
        if(time_IsFileOutdated(&modified, 7)) {
            uncompressed = gfx_MallocSprite(11, 11);
            dzx7_Standard(warning_compressed, uncompressed);
            gfx_Sprite(uncompressed, 16, 211);
            free(uncompressed);
        }
        ti_Close(avData);
    }
    else { gfx_PrintString("none"); }
    gfx_PrintStringXY("Definitions File: ", 30, 224);
    if((avData = ti_Open(VDefs, "r"))){
        time_struct_short_t modified;
        ti_Read(&modified, sizeof(time_struct_short_t), 1, avData);
        gfx_PrintUInt((int)modified.month, 2);
        gfx_PrintString("-");
        gfx_PrintUInt((int)modified.day, 2);
        gfx_PrintString("-");
        gfx_PrintUInt(modified.year, 4);
        if(time_IsFileOutdated(&modified, 30)) {
            uncompressed = gfx_MallocSprite(11, 11);
            dzx7_Standard(warning_compressed, uncompressed);
            gfx_Sprite(uncompressed, 16, 223);
            free(uncompressed);
        }
        ti_Close(avData);
    }
    else { gfx_PrintString("none"); }
    // Draw menu items
}


void pgrm_ApplySettings(void){
    if(settingsSave.hookInstall == 1) SetParserHook();
    if(settingsSave.hookInstall == 0) RemoveParserHook();
}

void pgrm_SaveSettings(void){
    ti_var_t settings;
    ti_CloseAll();
    if(settings = ti_Open(AVSettings, "w")){
        ti_Write(&settingsSave, sizeof(settings_save_t), 1, settings);
        ti_SetArchiveStatus(true, settings);
        ti_Close(settings);
    }
}



bool time_IsFileOutdated(time_struct_short_t *file, uint8_t maxAge){
    uint24_t systemdays, filedays;
    systemdays = (systemtime.year * 365) + (systemtime.month * 30) + systemtime.day;
    filedays = (file->year * 365) + (file->month * 30) + file->day;
    if(systemdays - filedays >= maxAge) return 1;
    return 0;
    
}


