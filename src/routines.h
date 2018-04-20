
#ifndef routines_h
#define routines_h

extern void av_CheckSumAll(void);
extern char av_ScanAll(void);
extern void pgrm_DrawSplashScreen(void);
extern char av_ValidateSaved(void);
extern void settings_SaveDate(void);
extern void settings_VerifyDate(void);
extern void SetHook();
extern void RemoveHook();
extern void pgrm_DrawSettingsMenu(void);
extern void pgrm_ApplySettings();
extern void pgrm_SaveSettings();
extern char pgrm_DrawMainMenu(void);


#endif
