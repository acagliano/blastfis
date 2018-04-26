#ifndef typedefs_h
#define typedefs_h
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

// Time Structures //
typedef struct {
    uint8_t sec, min, hour, day, month;
    uint16_t year; } time_struct_t;

typedef struct {
    uint8_t month, day;
    uint16_t year; } time_struct_short_t;

typedef struct {
    time_struct_t time;
    uint8_t hookInstall;
    uint8_t smartAttr;
    uint8_t enableFirewall;
    uint8_t enableQuarantine;
} settings_save_t;

// Opcode Temp Storage (for definitions parsing) //
typedef struct {
    char opcodestr[100];
    int opcodesize;
    char opcodehex[100];
    char desc[256];
} opitem_t;

// Program Attributes (for attribute update/verify) //
typedef struct {
    uint8_t type;
    uint16_t size;
    char name[9];
    uint24_t checksum;
} ProgData_t;


#endif
