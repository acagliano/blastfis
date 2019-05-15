#ifndef indexing_h
#define indexing_h

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
    int vers_track;
} progname_t;

#endif
