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
void* av_FileGetEnd(const char *name, uint8_t type);
size_t av_ShrinkFile(const char *name, uint8_t type, unsigned int sizealt);
void av_TogglePropTrack(progname_t* program);
void enable_PropTrack(progname_t* program);
void disable_PropTrack(progname_t* program);
void av_UpdateAttributes(progname_t* program);
uint16_t av_DeleteSnapshot(uint16_t index, uint16_t count);
void av_CreateSnapshot(progname_t* program);
void av_RestoreSnapshot(uint16_t index);
void av_UpdateSnapshot(progname_t* program, uint16_t index);
int16_t av_SnapDB_GetOpenSlotOffset(void);
int16_t av_SnapDB_GetSlotMatchOffset(progname_t* program);
snapshot_t* av_SnapDB_IndexToPtr(int16_t index);
#define av_SnapDB_LocateProgram(program) (av_SnapDB_IndexToPtr(av_SnapDB_GetSlotMatchOffset(program)))

#endif
