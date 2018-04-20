#include <graphx.h>
#include <fileioc.h>
#include <decompress.h>
#include "typedefs.h"
#include <string.h>
#include "globals.h"

void av_CheckSumAll(void){
    char *progname, tempread;
    time_struct_short_t modified;
    uint16_t ypos = 0;
    char searchtypes[2] = {TI_PRGM_TYPE, TI_PPRGM_TYPE}, i = 0;
    ti_var_t avData = ti_Open(ScanDB, "w");
    ti_CloseAll();
    
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

