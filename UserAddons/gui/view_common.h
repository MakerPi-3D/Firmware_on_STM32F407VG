#ifndef  VIEW_COMMON_H
#define  VIEW_COMMON_H
#include "wbtypes.h"
#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include "touch.h"
#include "lcd.h"
#include "view_pic_display.h"
#include "delay.h"
//#include  "ff.h"
#include "threed_engine.h"
#ifdef __cplusplus
extern "C" {
#endif

namespace gui_common
{
  extern volatile bool is_finish_print_beep_once;
};

///////////////////////////////////
#define MaxDisplayTextLength 23  //显示文件名时能显示的最大长度
#define DefaultTextSize 24

/////////////////////////////////////////////////
//  typedef unsigned          char   UCHAR;
//  typedef unsigned short     int   USHORT;
//  typedef unsigned           int   u32;
///////////////////////////////////////////////////
struct _textrange
{
  INT x;
  INT y;
  INT rangex;
  INT rangey;
};
//////////////////////////////////////////////////
struct _touch
{
  volatile  INT touch_flag;
  volatile  INT down_flag;
  volatile  INT up_flag;
  volatile  INT pixel_x;
  volatile  INT pixel_y;
  volatile  INT touch_x;
  volatile  INT touch_y;
};

extern struct _touch touch ;
extern void picturedisplay(PCHAR p);
////////////////////////////////////////////////////////
void SetTextDisplayRange(INT x, INT y, INT rangex, INT rangey, struct _textrange *p);
void ReadTextDisplayRangeInfo(struct _textrange textrange, PUSHORT p);
void CopyTextDisplayRangeInfo(struct _textrange textrange, CONST PUSHORT psource, PUSHORT p);
////////////////////////////////////////////////////////////////////
extern void DisplayTextInRange(PUCHAR pstr, struct _textrange textrange, PUSHORT p, INT size, uint16_t colour);
extern void DisplayTextInRangeDefault(PUCHAR pstr, struct _textrange textrange, PUSHORT p);
extern void DisplayText(PUCHAR pstr, INT x, INT y, INT size, uint16_t colour);
extern void DisplayTextDefault(PUCHAR pstr, INT x, INT y);
////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
typedef void (*menufunc_t)(void);
extern volatile menufunc_t currentdisplay;
extern volatile menufunc_t predisplay;
/////////////////////////////////////////////////////////
void  maindisplayF(void);
///////////////////////////////////////////////////////
INT IstouchxyDown(INT touchx, INT touchy, INT rangex, INT rangey); //add by zouxb
INT IstouchxyUp(void);  //add by zouxb
INT touchxy(INT touchx, INT touchy, INT rangex, INT rangey);
INT TouchXY_NoBeep(INT touchx, INT touchy, INT rangex, INT rangey);
INT touchrange(INT touchx, INT touchy, INT rangex, INT rangey);

extern void gui_init_lcd_touch(void);
extern void gui_set_curr_display(const volatile menufunc_t display);
extern void gui_display_touch_ctrl(void);
extern uint8_t gui_is_refresh(void);
extern uint8_t gui_is_rtc(void);
extern void change_print_file_name(void);

//////////////////////////////////////////////////////
extern volatile INT print_flage;
extern volatile INT pauseprint;
//  extern INT keysound;
//  extern INT alarmsound;

///////////////////////////////////////////////////////////

#ifndef MK_MAX_LFN
#define MK_MAX_LFN     128  /* Maximum LFN length to handle (12 to 255) */
#endif
extern char printname[MK_MAX_LFN];
extern char printnameb[MK_MAX_LFN];            //打印文件的名称
extern INT page;
//////////////////////////////////////////////////////////


extern struct _textrange  NozzleTempTextRange;    //喷嘴温度的显示区域
extern struct _textrange  HotBedTempTextRange;    //热床温度的显示区域
extern struct _textrange  NozzleTargetTempTextRange;   //喷嘴目标温度的显示区域
extern struct _textrange  HotBedTargetTempTextRange;   //热床目标温度的显示区域
extern struct _textrange  PrintScheduleTextRange;    //打印进度的显示区域
extern struct _textrange  PrintTimeTextRange;    //打印时间的显示区域
extern struct _textrange  SpeedRange;    //速度的显示区域
extern struct _textrange  CavityTempTextRange;    //腔体温度的显示区域
extern struct _textrange  CavityTargetTempTextRange;   //腔体目标温度的显示区域
extern struct _textrange  RunTimeTextRange; // 运行时间的显示区域
extern struct _textrange  PrintSpeedTextSharp; //打印速度的显示区域
extern struct _textrange  FanSpeedTextSharp; //风扇速度的显示区域
extern struct _textrange  LedSwitchTextSharp;
extern struct _textrange  CavityTempOnTextSharp; //腔体温度开关的显示区域
extern struct _textrange  ZOffsetZeroTextSharp; //Z零点偏移的显示区域
extern struct _textrange  XPosTextShape; //X位置的显示区域
extern struct _textrange  YPosTextShape; //Y位置的显示区域
extern struct _textrange  ZPosTextShape; //Z位置的显示区域


#ifdef __cplusplus
} //extern "C" {
#endif

/////////////////////////////////////////
#endif //VIEW_COMMON_H

