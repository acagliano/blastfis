#include <graphx.h>
#include <fileioc.h>
#include <decompress.h>
#include "typedefs.h"
#include "globals.h"
#include <string.h>


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

