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
    bool indexed;
    char type;
    char name[9];
    unsigned long checksum;
    unsigned int size;
    unsigned int prop_track;
} progname_t;

typedef struct {
    char type;
    char fname[9];
    size_t size;
    stime_t time;
} snapshot_t;

typedef struct {
    bool indexSplit;
} settings_t;

#endif
