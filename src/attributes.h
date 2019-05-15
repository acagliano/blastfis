#ifndef attributes_h
#define attributes_h
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>



// Program Attributes (for attribute update/verify) //
typedef struct {
    char name[9];
    uint8_t type;
    uint24_t size;
    uint8_t* snapshot;
    unsigned long checksum;
} attributes_t;


#endif
