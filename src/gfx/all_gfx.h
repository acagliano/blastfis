// convpng v7.0
// this file contains all the graphics sources for easy inclusion in a project
#ifndef __all_gfx__
#define __all_gfx__
#include <stdint.h>

#define warning_width 11
#define warning_height 11
#define warning_size 123
extern uint8_t warning_compressed[38];
#define tracking_icon_width 9
#define tracking_icon_height 9
#define tracking_icon_size 83
extern uint8_t tracking_icon_compressed[35];
#define blast_icon_width 60
#define blast_icon_height 60
#define blast_icon_size 3602
extern uint8_t blast_icon_compressed[384];
#define integ_pass_icon_width 9
#define integ_pass_icon_height 9
#define integ_pass_icon_size 83
extern uint8_t integ_pass_icon_compressed[31];
#define integ_fail_icon_width 9
#define integ_fail_icon_height 9
#define integ_fail_icon_size 83
extern uint8_t integ_fail_icon_compressed[33];
#define sizeof_all_gfx_pal 512
extern uint16_t all_gfx_pal[256];

#endif
