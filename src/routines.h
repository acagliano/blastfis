
#ifndef routines_h
#define routines_h

void av_CheckSumAll(void);
char av_ScanAll(void);
void pgrm_DrawSplashScreen(void);
char av_ValidateSaved(void);
void settings_SaveDate(void);
void settings_VerifyDate(void);
extern void SetHook();
void RemoveHook();
void pgrm_DrawSettingsMenu(void);
void pgrm_ApplySettings();
void pgrm_SaveSettings();
char pgrm_DrawMainMenu(char option);
void showBoxes(void);
bool time_IsFileOutdated(time_struct_short_t *file, uint8_t maxAge);


#endif
