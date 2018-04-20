#ifndef splash_h
#define splash_h

#include <tice.h>

bool time_IsFileOutdated(time_struct_short_t *file, uint8_t maxAge);

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
    gfx_PrintStringXY("Firewall State: Offline", 30, 200);
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
            gfx_Sprite(uncompressed, 16, 210);
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
            gfx_Sprite(uncompressed, 16, 222);
            free(uncompressed);
        }
        ti_Close(avData);
    }
    else { gfx_PrintString("none"); }
    // Draw menu items
}

bool time_IsFileOutdated(time_struct_short_t *file, uint8_t maxAge){
    uint24_t systemdays, filedays;
    systemdays = (systemtime.year * 365) + (systemtime.month * 30) + systemtime.day;
    filedays = (file->year * 365) + (file->month * 30) + file->day;
    if(systemdays - filedays >= maxAge) return 1;
    return 0;
    
}

#endif
