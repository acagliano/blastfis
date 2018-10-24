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
    bool enable_TimeRestore;
    bool enable_FileQuarantine;
} settings_save_t;

// Opcode Temp Storage (for definitions parsing) //
typedef struct {
    char opcodestr[100];
    int opcodesize;
    char opcodehex[100];
} opitem_t;

// Program Attributes (for attribute update/verify) //
typedef struct {
    uint8_t type;
    uint24_t size;
    char name[9];
    unsigned long checksum;
} ProgData_t;

typedef struct {
    char progtype;
    char progname[9];
} program_t;

#endif
