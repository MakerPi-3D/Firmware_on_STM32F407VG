#ifndef LCD_ILI9488_H
#define LCD_ILI9488_H
#include "wbtypes.h"
//#include "stm32f4xx_hal.h"
#ifdef __cplusplus
extern "C" {
#endif
  extern void ILI9488_Lcd_Init(void);
  extern void ILI9488_Fsmc_ReInit(void);
  extern UINT16 ILI9488_Get_IC_ID(void);
  extern void ILI9488_Lcd_SetCursor(UINT16 Xcmd, UINT16 Ycmd, UINT16 Xpos, UINT16 Ypos, UINT16 width, UINT16 height);
  extern void ILI9488_Lcd_Fill(UINT16 Xcmd, UINT16 Ycmd, UINT16 sx, UINT16 sy, UINT16 ex, UINT16 ey, UINT16 color);
  extern void ILI9488_Lcd_Color_Fill(UINT16 Xcmd, UINT16 Ycmd, UINT16 sx, UINT16 sy, UINT16 ex, UINT16 ey, const UINT16* color);
  extern void ILI9488_Lcd_Scan_Dir(UINT16 width, UINT16 height, UINT16 Xcmd, UINT16 Ycmd, UINT8 dir);
#ifdef __cplusplus
} //extern "C"
#endif

#endif // LCD_ILI9488_H



