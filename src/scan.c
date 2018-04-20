#include <graphx.h>
#include <fileioc.h>
#include "typedefs.h"
#include "globals.h"
#include <string.h>

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


