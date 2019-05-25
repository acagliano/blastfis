#ifndef avfuncs_h
#define avfuncs_h

#include "indexing.h"
extern const char *PropDB;
extern const char *AvDB;
extern const char *SnapDB;
extern const char *AVSettings;
extern const char *OSProp;
extern const char *AVDefs;
#define TI_SNAPSHOT_TYPE 0xff

uint24_t av_GetNumFiles(void);
uint16_t av_GetSnapsCount(void);
int av_GenerateFileIndex(progname_t* prognames, uint24_t count);
void av_GenerateSnapIndex(snapname_t* snapnames, uint16_t count);
void av_TellAttributes(progname_t* program);
progsave_t* av_LocateFileInPropDB(progname_t* program);
void av_CollapseDB(progsave_t* delete);
void* av_FileGetPtr(const char *name, uint8_t type, size_t *Len);
void* av_FileGetEnd(const char *name, uint8_t type);
size_t av_ShrinkFile(const char *name, uint8_t type, unsigned int sizealt);
void av_TogglePropTrack(progname_t* program);
void enable_PropTrack(progname_t* program);
void disable_PropTrack(progname_t* program);
void av_UpdateAttributes(progname_t* program);
void av_DeleteSnapshot(snapname_t* snapname);
void av_CreateSnapshot(snapname_t* snapnames, uint16_t num_snaps, progname_t* program);
void av_UpdateSnapshot(snapname_t* snapnames, uint16_t count, progname_t* program);
void av_RestoreSnapshot(snapname_t* snapname, uint16_t which);
int24_t av_FindSnap(snapname_t* snapnames, size_t count, progname_t* program);
void av_ChecksumOS(ossave_t* ossave, char *start, char *end);
void av_SaveOSAttr(ossave_t* ossave);
void av_ScanData(char* data, size_t size, gfx_sprite_t* warning);
uint24_t av_FindScanMatch(char* data, size_t datasize, char* readbuff, size_t len);
#define av_SnapDB_LocateProgram(program) (av_SnapDB_IndexToPtr(av_SnapDB_GetSlotMatchOffset(program)))

#endif
