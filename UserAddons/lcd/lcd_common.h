#ifndef LCD_COMMON_H
#define LCD_COMMON_H
#include "stm32f4xx_hal.h"
#include "wbtypes.h"
#ifdef __cplusplus
extern "C" {
#endif

//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左

#define DFT_SCAN_DIR  R2L_D2U

  extern SRAM_HandleTypeDef hsram1;
  extern void LCD_WR_REG(__IO UINT16 regval);
  extern void LCD_WR_DATA(__IO UINT16 data);
  extern UINT16 LCD_RD_DATA(void);
  extern void LCD_WriteRAM_Prepare(void);
  extern void LCD_WriteReg(__IO UINT16 LCD_Reg, __IO UINT16 LCD_RegValue);
  extern UINT16 LCD_ReadReg(__IO UINT16 LCD_Reg);
  extern UINT16 LCD_RD_REG(void);

#ifdef __cplusplus
} //extern "C"
#endif
#endif // LCD_COMMON_H

