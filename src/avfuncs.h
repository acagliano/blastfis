#ifndef avfuncs_h
#define avfuncs_h

#include "indexing.h"
extern const char *PropDB;
extern const char *AvDB;

int av_GetNumFiles(void);
int av_GenerateFileIndex(progname_t* prognames, int count);
progsave_t* av_LocateFileInDB(progname_t* program);
void av_CollapseDB(progsave_t* delete);
void* av_FileGetPtr(char *name, int *Len);
void* av_ResizeFile(char *name, uint8_t type, int sizealt)
void av_TogglePropTrack(progname_t* program);
void enable_PropTrack(progname_t* program);
void disable_PropTrack(progname_t* program);

#endif
