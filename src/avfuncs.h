#ifndef avfuncs_h
#define avfuncs_h

#include "indexing.h"
extern const char *PropDB;
extern const char *AvDB;
extern const char *SnapDB;

int av_GetNumFiles(void);
int av_GenerateFileIndex(progname_t* prognames, int count);
progsave_t* av_LocateFileInPropDB(progname_t* program);
void av_CollapseDB(progsave_t* delete);
void* av_FileGetPtr(const char *name, uint8_t type, size_t *Len);
size_t av_ShrinkFile(const char *name, uint8_t type, unsigned int sizealt);
void av_TogglePropTrack(progname_t* program);
void enable_PropTrack(progname_t* program);
void disable_PropTrack(progname_t* program);
void av_UpdateAttributes(progname_t* program);
void av_ToggleSnapshot(progname_t* program);
void av_UpdateSnapshot(progname_t* program);
void av_DeleteSnapshot(progname_t* program);
void av_CreateSnapshot(progname_t* program);
unsigned int av_LocateFileInSnapDB(progname_t* program);

#endif
