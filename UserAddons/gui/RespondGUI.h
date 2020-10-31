#ifndef RESPONDGUI_H
#define RESPONDGUI_H

#include "wbtypes.h"
//#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MinTempWarning 0U
#define MaxTempWarning 1U
#define MaxTempBedWarning 2U
#define HeatFailWarning 3U
#define XMinLimitWarning 4U
#define YMinLimitWarning 5U
#define ZMinLimitWarning 6U
#define XMaxLimitWarning 7U
#define YMaxLimitWarning 8U
#define ZMaxLimitWarning 9U
#define IWDGResetWarning 10U
#define FatfsWarning 11U
#define ThermistorFallsWarning 12U

  void PopWarningInfo(UINT8 WarningSelectValue);
  void ManagWarningInfo(void);
//void PopPromptInfo(UINT8 PromptInfoSelectValue , CONST PCHAR TextInfo);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif //RESPONDGUI_H

