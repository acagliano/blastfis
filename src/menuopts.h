#ifndef menuopts_h
#define menuopts_h

enum Screens {
    FILE_OPTS,
    SYS_SCANS,
    SNAPSHOTS,
    SETTINGS,
    ABOUT,
    QUIT,
    NUM_SCREENS,
    MAIN
};

enum ProgOpts {
    NONE,
    TRACK_TOGG,
    TRACK_UPD,
    SCAN,
    SNAP_ENABLE
};

typedef struct {
    int x, y;
} progopt_t;

typedef struct {
    unsigned char menu;
    unsigned char program;
    unsigned char progopt;
    unsigned char snapopt;
    uint16_t snapshot;
} selected_t;

#endif
