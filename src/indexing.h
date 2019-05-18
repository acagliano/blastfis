#ifndef indexing_h
#define indexing_h
#include <stddef.h>
#include "timestamps.h"


typedef struct {
    char type;
    char name[9];
    unsigned long checksum;
    int size;
} progsave_t;

typedef struct {
    char type;
    char name[9];
    unsigned long checksum;
    unsigned int size;
    unsigned int prop_track;
} progname_t;

typedef struct {
    size_t size;
    char type;
    char fname[9];
    stime_t time;
    uint8_t data[1];
} snapshot_t;


#endif
