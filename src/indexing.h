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
    size_t size;
    unsigned long checksum;
} ossave_t;

typedef struct {
    bool indexed;
    char type;
    char name[9];
    unsigned long checksum;
    unsigned int size;
    unsigned int prop_track;
} progname_t;

typedef struct {
    size_t savesize;
    char type;
    char name[9];
    size_t size;
    stime_t time;
    char data[1];
} snapshot_t;

typedef struct {
    char snapname[9];
    char progname[9];
    uint8_t progtype;
} snapname_t;

typedef struct {
    bool indexSplit;
    uint16_t maxSnaps;
} settings_t;

#endif
