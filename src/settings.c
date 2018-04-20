#include <graphx.h>
#include <fileioc.h>
#include <decompress.h>
#include "typedefs.h"
#include "splash.c"
#include "routines.h"
void showBoxes();

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
        gfx_PrintStringXY("Enable Firewall", 130, 124);
        gfx_PrintStringXY("Enable Quarantine", 130, 136);
        gfx_PrintStringXY("Main Menu", 130, 148);
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
                if(selected < 1) selected = 4;
            }
            if(key == sk_Down){
                selected++;
                if(selected > 4) selected = 1;
            }
            if(key == sk_Enter){
                switch(selected){
                    case 1:
                        settingsSave.hookInstall = !settingsSave.hookInstall;
                        showBoxes();
                        break;
                    case 2:
                        settingsSave.enableFirewall = !settingsSave.enableFirewall;
                        showBoxes();
                        break;
                    case 3:
                        settingsSave.enableQuarantine = !settingsSave.enableQuarantine;
                        showBoxes();
                        break;
                    case 4:
                        key = sk_Clear;
                        break;
                }
            }
            if(key) gfx_FillCircle(120, selected * 12 + 103, 3);
            if(key == sk_Clear) { pgrm_SaveSettings(); pgrm_ApplySettings(); }
        }
}

void pgrm_ApplySettings(){
    if(settingsSave.hookInstall == 1) SetHook();
    if(settingsSave.hookInstall == 0) RemoveHook();
}

void pgrm_SaveSettings(){
    char key = 0;
    ti_var_t settings;
    ti_CloseAll();
    if(settings = ti_Open(AVSettings, "w")){
        ti_Write(&settingsSave, sizeof(settings_save_t), 1, settings);
        ti_SetArchiveStatus(true, settings);
        ti_Close(settings);
    } else {
        gfx_PrintString("failed to open file");
        while(os_GetCSC() != sk_Enter);
    }
}

void showBoxes(){
    gfx_sprite_t *uncompressed_off, *uncompressed_on;
    uncompressed_off = gfx_MallocSprite(8,8);
    dzx7_Standard(funcoff_compressed, uncompressed_off);
    uncompressed_on = gfx_MallocSprite(8,8);
    dzx7_Standard(funcon_compressed, uncompressed_on);
    if(settingsSave.hookInstall){ gfx_Sprite(uncompressed_on, 280, 112); }
    else { gfx_Sprite(uncompressed_off, 280, 112); }
    if(settingsSave.enableFirewall){ gfx_Sprite(uncompressed_on, 280, 124); }
    else {gfx_Sprite(uncompressed_off, 280, 124);}
    if(settingsSave.enableQuarantine) {gfx_Sprite(uncompressed_on, 280, 136);}
    else {gfx_Sprite(uncompressed_off, 280, 136);}
    free(uncompressed_on);
    free(uncompressed_off);
}
