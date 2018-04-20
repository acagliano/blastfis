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

/* Put your function prototypes here */



#include "routines.h"
#include "gfx/all_gfx.h"
/* Put all your globals here. */
#include "globals.h"


void main(void) {
	/* Fill in the body of the main function here */
    char option;
    ti_var_t settings;
    time_struct_t *savedtime = &settingsSave.savedtime;
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
    while((option = pgrm_DrawMainMenu()) != 6){
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
