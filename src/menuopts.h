#ifndef menuopts_h
#define menuopts_h

enum Screens {
    FILE_OPTS,
    SYS_SCANS,
    SETTINGS,
    ABOUT,
    QUIT,
    NUM_SCREENS,
    MAIN
};

enum ProgOpts {
    NONE,
    TRACK_TOGG,
    SNAP_TOGG,
    TRACK_UPD,
    SNAP_UPD,
    SCAN
};

typedef struct {
    int x, y;
} progopt_t;

typedef struct {
    unsigned char menu;
    unsigned char program;
    unsigned char progopt;
} selected_t;

#endif
