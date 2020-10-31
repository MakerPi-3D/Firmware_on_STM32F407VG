#ifndef __LCD_H
#define __LCD_H
#include "wbtypes.h"
#include <stdint.h>
//#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct
  {
    UINT16  width;			// LCD 宽度
    UINT16  height;			// LCD 高度
    UINT16  id;				  // LCD ID
    UINT16	wramcmd;		// 开始写gram指令
    UINT16  setxcmd;		// 设置x坐标指令
    UINT16  setycmd;		// 设置y坐标指令
    UINT8   dir;			  // 横屏还是竖屏控制：0，竖屏；1，横屏。
  } _lcd_dev;

//LCD参数
  extern _lcd_dev lcddev;	    // 管理LCD重要参数
//LCD的画笔颜色和背景色
  extern UINT16 POINT_COLOR;// 默认红色
  extern UINT16 BACK_COLOR; // 背景颜色.默认为白色

#define X_1963(x) x=(UINT16)(x*0.85f)+36
#define Y_1963(y) y=(UINT16)(y*0.85f)

//////////////////////////////////////////////////////////////////////////////////

//画笔颜色
#define WHITE      0xFFFF
#define BLACK      0x0000
#define BLUE       0x001F
#define BRED       0XF81F
#define GRED       0XFFE0
#define GBLUE      0X07FF
#define RED        0xF800
#define MAGENTA    0xF81F
#define GREEN      0x07E0
#define CYAN       0x7FFF
#define YELLOW     0xFFE0
#define BROWN      0XBC40 // 棕色
#define BRRED      0XFC07 // 棕红色
#define GRAY       0X8430 // 灰色
//GUI颜色
#define DARKBLUE   0X01CF // 深蓝色
#define LIGHTBLUE  0X7D7C // 浅蓝色  
#define GRAYBLUE   0X5458 // 灰蓝色
#define BACKBLUE   0X0AF7 //当前使用图片的背景蓝
//以上三色为PANEL的颜色
#define LIGHTGREEN 0X841F //浅绿色
//#define LIGHTGRAY  0XEF5B //浅灰色(PANNEL)
#define LGRAY      0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE  0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE     0X2B12 //浅棕蓝色(选择条目的反色)

#define DEFAULT_DRAW_COLOR RED

  void LCD_Init(void);                                                                                  //初始化
  void LCD_ReInit(void);
  void LCD_DisplayOn(void);                                                                             //开显示
  void LCD_DisplayOff(void);                                                                            //关显示
  void LCD_Clear(UINT16 Color);                                                                       //清屏
  void LCD_SetCursor(UINT16 Xpos, UINT16 Ypos);                                                     //设置光标
  void LCD_DrawPoint(UINT16 x,UINT16 y);                                                            //画点
  void LCD_Fast_DrawPoint(UINT16 x,UINT16 y,UINT16 color);                                        //快速画点
  UINT16 LCD_ReadPoint(UINT16 x,UINT16 y);                                                        //读点
  void LCD_Draw_Circle(UINT16 x0,UINT16 y0,UINT8 r);                                              //画圆
  void LCD_DrawLine(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2);                                //画线
  void LCD_DrawRectangle(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2);                           //画矩形
  void LCD_Fill(UINT16 sx,UINT16 sy,UINT16 ex,UINT16 ey, uint16_t color);                        //填充单色
  void LCD_Fill_Default(UINT16 sx,UINT16 sy,UINT16 ex,UINT16 ey);                        //填充单色
  void LCD_Color_Fill(UINT16 sx,UINT16 sy,UINT16 ex,UINT16 ey,const UINT16 *color);           //填充指定颜色
#if USE_LCD_SHOW
  void LCD_ShowChar(UINT16 x,UINT16 y,UINT8 num,UINT8 size,UINT8 mode);                       //显示一个字符
  void LCD_ShowNum(UINT16 x,UINT16 y,UINT32 num,UINT8 len,UINT8 size);                        //显示一个数字
  void LCD_ShowxNum(UINT16 x,UINT16 y,UINT32 num,UINT8 len,UINT8 size,UINT8 mode);          //显示 数字
  void LCD_ShowString(UINT16 x,UINT16 y,UINT16 width,UINT16 height,UINT8 size,UINT8 *p);    //显示一个字符串,12/16字体
#endif
  void LCD_Scan_Dir(UINT8 dir);                                                                       //设置屏扫描方向
  void LCD_Display_Dir(UINT8 dir);                                                                    //设置屏幕显示方向
  void LCD_Set_Window(UINT16 sx,UINT16 sy,UINT16 width,UINT16 height);                          //设置窗口
  void LCD_Fill_Picture(UINT16 sx,UINT16 sy,UINT16 ex,UINT16 ey,UINT16 color);                //画图专用，SSD1963画图与填充单色需分开

//9320/9325 LCD寄存器
#define R0             0x00
#define R1             0x01
#define R2             0x02
#define R3             0x03
#define R4             0x04
#define R5             0x05
#define R6             0x06
#define R7             0x07
#define R8             0x08
#define R9             0x09
#define R10            0x0A
#define R12            0x0C
#define R13            0x0D
#define R14            0x0E
#define R15            0x0F
#define R16            0x10
#define R17            0x11
#define R18            0x12
#define R19            0x13
#define R20            0x14
#define R21            0x15
#define R22            0x16
#define R23            0x17
#define R24            0x18
#define R25            0x19
#define R26            0x1A
#define R27            0x1B
#define R28            0x1C
#define R29            0x1D
#define R30            0x1E
#define R31            0x1F
#define R32            0x20
#define R33            0x21
#define R34            0x22
#define R36            0x24
#define R37            0x25
#define R40            0x28
#define R41            0x29
#define R43            0x2B
#define R45            0x2D
#define R48            0x30
#define R49            0x31
#define R50            0x32
#define R51            0x33
#define R52            0x34
#define R53            0x35
#define R54            0x36
#define R55            0x37
#define R56            0x38
#define R57            0x39
#define R59            0x3B
#define R60            0x3C
#define R61            0x3D
#define R62            0x3E
#define R63            0x3F
#define R64            0x40
#define R65            0x41
#define R66            0x42
#define R67            0x43
#define R68            0x44
#define R69            0x45
#define R70            0x46
#define R71            0x47
#define R72            0x48
#define R73            0x49
#define R74            0x4A
#define R75            0x4B
#define R76            0x4C
#define R77            0x4D
#define R78            0x4E
#define R79            0x4F
#define R80            0x50
#define R81            0x51
#define R82            0x52
#define R83            0x53
#define R96            0x60
#define R97            0x61
#define R106           0x6A
#define R118           0x76
#define R128           0x80
#define R129           0x81
#define R130           0x82
#define R131           0x83
#define R132           0x84
#define R133           0x85
#define R134           0x86
#define R135           0x87
#define R136           0x88
#define R137           0x89
#define R139           0x8B
#define R140           0x8C
#define R141           0x8D
#define R143           0x8F
#define R144           0x90
#define R145           0x91
#define R146           0x92
#define R147           0x93
#define R148           0x94
#define R149           0x95
#define R150           0x96
#define R151           0x97
#define R152           0x98
#define R153           0x99
#define R154           0x9A
#define R157           0x9D
#define R192           0xC0
#define R193           0xC1
#define R229           0xE5

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
