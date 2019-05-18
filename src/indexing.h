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
    int size;
    unsigned int prop_track;
    bool snapshot;
} progname_t;

typedef struct {
    char type;
    char fname[9];
    stime_t time;
    size_t size;
    uint8_t data[1];
} snapshot_t;


#endif
