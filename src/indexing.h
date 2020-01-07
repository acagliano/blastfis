#ifndef indexing_h
#define indexing_h
#include <stddef.h>
#include "timestamps.h"


typedef struct {
    char type;
    char name[9];
    unsigned int checksum;
    size_t size;
} progsave_t;

typedef struct {
    size_t size;
    unsigned int checksum;
} ossave_t;

typedef struct {
    bool indexed;
    char type;
    char name[9];
    unsigned int checksum;
    size_t size;
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
    bool indexProgs, indexAppvars;
    uint16_t maxSnaps;
} settings_t;

#endif
