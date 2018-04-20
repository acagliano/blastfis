

char pgrm_DrawMainMenu(void){
    char selected = 1, key = 0;
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
