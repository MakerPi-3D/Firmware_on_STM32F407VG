#ifndef __LCD_SSD1963_H
#define __LCD_SSD1963_H

#include "stm32f4xx_hal.h"
#ifdef __cplusplus
extern "C" {
#endif

//定义LCD的尺寸
#define LCD_W 480
#define LCD_H 272
#define SSD1963_ID 0X6101 //获取到的IC值

//#define X_1963(x) x=(uint16_t)(x*0.85f)+36
//#define Y_1963(y) y=(uint16_t)(y*0.85f)

  uint16_t SSD1963_Get_IC_ID(void);
  void SSD1963_Lcd_Init(void);
  void SSD1963_Lcd_SetCursor(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
  void SSD1963_Lcd_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);
  void SSD1963_Lcd_Color_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend, const uint16_t* color);
  void SSD1963_Lcd_Scan_Dir(uint16_t width, uint16_t height, uint16_t Xcmd, uint16_t Ycmd, uint8_t dir);


#ifdef __cplusplus
} //extern "C" {
#endif

#endif //#define __LCD_SSD1963_H	




