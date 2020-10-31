#ifndef LCD_NT35310_H
#define LCD_NT35310_H
#include "wbtypes.h"
//#include "stm32f4xx_hal.h"
#ifdef __cplusplus
extern "C" {
#endif
  extern void NT35310_Lcd_Init(void);
  extern void NT35310_Fsmc_ReInit(void);
  extern UINT16 NT35310_Get_IC_ID(void);
  extern void NT35310_Lcd_SetCursor(UINT16 Xcmd, UINT16 Ycmd, UINT16 Xpos, UINT16 Ypos);
  extern void NT35310_Lcd_Fill(UINT16 Xcmd, UINT16 Ycmd, UINT16 sx, UINT16 sy, UINT16 ex, UINT16 ey, UINT16 color);
  extern void NT35310_Lcd_Color_Fill(UINT16 Xcmd, UINT16 Ycmd, UINT16 sx, UINT16 sy, UINT16 ex, UINT16 ey, const UINT16* color);
  extern void NT35310_Lcd_Scan_Dir(UINT16 width, UINT16 height, UINT16 Xcmd, UINT16 Ycmd, UINT8 dir);
#ifdef __cplusplus
} //extern "C"
#endif

#endif // LCD_NT35310_H


