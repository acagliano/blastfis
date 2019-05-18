
#ifndef timestamps_h
#define timestamps_h

// Time Structures //
typedef struct {
    uint8_t sec, min, hour, day, month;
    uint16_t year; } ltime_t;

typedef struct {
    uint8_t month, day;
    uint16_t year; } stime_t;

#endif
