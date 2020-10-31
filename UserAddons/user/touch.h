#ifndef __TOUCH_H__
#define __TOUCH_H__

#include "wbtypes.h"
//#include "sys.h"
//#include "stm32f4xx.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//触摸屏驱动（支持ADS7843/7846/UH7843/7846/XPT2046/TSC2046/OTT2001A等） 代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/7
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//********************************************************************************
//修改说明
//V1.1 20140721
//修正MDK在-O2优化时,触摸屏数据无法读取的bug.在TP_Write_Byte函数添加一个延时,解决问题.
//////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

//#define LCD_DEBUG //卢工调试LCD要求


#define TP_PRES_DOWN 0x80  //触屏被按下	  
#define TP_CATH_PRES 0x40  //有按键按下了 

#define OTT_MAX_TOUCH  5

//触摸屏控制器
typedef struct
{
  UINT8 (*init)(void);			//初始化触摸屏控制器
  UINT8 (*scan)(UINT8);				//扫描触摸屏.0,屏幕扫描;1,物理坐标;
  void (*adjust)(void);		//触摸屏校准
  UINT16 x[OTT_MAX_TOUCH]; 		//当前坐标
  UINT16 y[OTT_MAX_TOUCH];		//电容屏有最多5组坐标,电阻屏则用x[0],y[0]代表:此次扫描时,触屏的坐标,用
  //x[4],y[4]存储第一次按下时的坐标.
  UINT8  sta;					//笔的状态
  //b7:按下1/松开0;
  //b6:0,没有按键按下;1,有按键按下.
  //b5:保留
  //b4~b0:电容触摸屏按下的点数(0,表示未按下,1表示按下)
/////////////////////触摸屏校准参数(电容屏不需要校准)//////////////////////
  FLOAT xfac;
  FLOAT yfac;
  SHORT xoff;
  SHORT yoff;
//新增的参数,当触摸屏的左右上下完全颠倒时需要用到.（20171104上下左右理解：若以默认设置b0：0，显示方向为横屏为方向。则左右为屏幕的长，上下为屏幕的宽）
//b0:0,竖屏(适合左右为X坐标,上下为Y坐标的TP)
//   1,横屏(适合左右为Y坐标,上下为X坐标的TP)
//b1~6:保留.(xfac为负数，b1置1；yfac为负数，b2置1)
//b7:0,电阻屏
//   1,电容屏
  UINT8 touchtype;
} _m_tp_dev;

extern _m_tp_dev tp_dev;	 	//触屏控制器在touch.c里面定义

////电阻屏芯片连接引脚
//#define PEN  		PBin(1)  	//T_PEN
//#define DOUT 		PBin(2)   	//T_MISO

////#define TDIN 		PFout(11)  	//T_MOSI
////////////////////////////////////////////////////////////////
//#define TDIN 		PCout(1)  	//T_MOSI
//////////////////////////////////////////////////////////////
//#define TCLK 		PBout(0)  	//T_SCK
//#define TCS  		PCout(13)  	//T_CS

//电阻屏函数
void TP_Write_Byte(UINT8 num);						//向控制芯片写入一个数据
UINT16 TP_Read_AD(UINT8 CMD);							//读取AD转换值
UINT16 TP_Read_XOY(UINT8 xy);							//带滤波的坐标读取(X/Y)
UINT8 TP_Read_XY(UINT16 *x,UINT16 *y);					//双方向读取(X+Y)
UINT8 TP_Read_XY2(UINT16 *x,UINT16 *y);					//带加强滤波的双方向坐标读取
void TP_Drow_Touch_Point(UINT16 x,UINT16 y,UINT16 color);//画一个坐标校准点
void TP_Draw_Big_Point(UINT16 x,UINT16 y,UINT16 color);	//画一个大点
void TP_Save_Adjdata(void);						//保存校准参数
UINT8 TP_Get_Adjdata(void);						//读取校准参数
void TP_Adjust(void);							//触摸屏校准
//电阻屏/电容屏 共用函数
UINT8 TP_Scan(UINT8 tp);								//扫描
UINT8 TP_Init(void);								//初始化



#ifdef __cplusplus
} //extern "C" {
#endif

#endif

















