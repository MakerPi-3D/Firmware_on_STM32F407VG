#include "lcd.h"
#include "lcd_common.h"
#include "lcd_nt35310.h"
#include "lcd_ili9488.h"
#include "LCD_SSD1963.h"
#include "stm32f4xx_hal.h"
#include "sysconfig_data.h"
#include "config_model_tables.h"
#include "globalvariables.h"
#define NT35310_ID 0X5310
#define ILI9488_ID 0X9488
//#include "delay.h"
#include "font.h"
//#include "user_debug.h"

//LCD的画笔颜色和背景色
UINT16 POINT_COLOR=0x0000; //画笔颜色
UINT16 BACK_COLOR=0xFFFF;  //背景色

//管理LCD重要参数
_lcd_dev lcddev;
extern SRAM_HandleTypeDef hsram1;

#if USE_LCD_SHOW
static UINT32 LCD_Pow(UINT8 m,UINT8 n);
#endif
//static UINT16 LCD_BGR2RGB(UINT16 c);
static void LCD_Get_IC_ID(void);
//static void opt_delay(UINT8 i);



//初始化lcd
//lcd型号：NT35310
void LCD_Init(void)
{
  LCD_Get_IC_ID(); //读id
//  USER_DbgLog("LCD IC ID::0x%04x\n",lcddev.id);
  if((0X5310 != lcddev.id) && (0X9488 != lcddev.id) && (SSD1963_ID != lcddev.id))
  {
    HAL_Delay(100);
    LCD_Get_IC_ID();
    if((0X5310 != lcddev.id) && (0X9488 != lcddev.id) && (SSD1963_ID != lcddev.id))
    {
      if(K5==t_sys_data_current.model_id)
        lcddev.id = SSD1963_ID;
      else
        lcddev.id = 0X5310;
    }
//    USER_DbgLog("LCD IC ID::0x%04x\n",lcddev.id);
  }
  if(lcddev.id==NT35310_ID)
  {
    NT35310_Fsmc_ReInit();
    NT35310_Lcd_Init();
  }
  else if(lcddev.id==ILI9488_ID)
  {
    ILI9488_Fsmc_ReInit();
    ILI9488_Lcd_Init();
  }
  else if(lcddev.id==SSD1963_ID)
  {
    ILI9488_Fsmc_ReInit();
    SSD1963_Lcd_Init();
  }
  LCD_Display_Dir(1);//横屏
}

//初始化lcd
//lcd型号：NT35310
void LCD_ReInit(void)
{
  LCD_Get_IC_ID(); //读id
//  USER_DbgLog("LCD IC ID::0x%04x\n",lcddev.id);
  if((0X5310 != lcddev.id) && (0X9488 != lcddev.id))
  {
    HAL_Delay(100);
    LCD_Get_IC_ID();
    if((0X5310 != lcddev.id) && (0X9488 != lcddev.id))
      lcddev.id = 0X5310;
//    USER_DbgLog("LCD IC ID::0x%04x\n",lcddev.id);
  }
  LCD_Display_Dir(1);//横屏
  if(lcddev.id==NT35310_ID)
  {
    NT35310_Lcd_Init();
  }
  else if(lcddev.id==ILI9488_ID)
  {
    ILI9488_Lcd_Init();
  }
  LCD_Display_Dir(1);//横屏
}


/**
 * @brief LCD_DisplayOn LCD开启显示
 */
void LCD_DisplayOn(void)
{
  if(lcddev.id==NT35310_ID)
    LCD_WR_REG(0X29);	//开启显示
}

/**
 * @brief LCD_DisplayOff LCD关闭显示
 */
void LCD_DisplayOff(void)
{
  if(lcddev.id==NT35310_ID)
    LCD_WR_REG(0X28);	//关闭显示
}

/**
 * @brief LCD_Clear 清屏函数
 * @param color 要清屏的填充色
 */
void LCD_Clear(UINT16 color)
{
  UINT32 index=0;
  UINT32 totalpoint=lcddev.width;
  totalpoint*=lcddev.height; 			//得到总点数
  if((lcddev.id==0X6804)&&(lcddev.dir==1))//6804横屏的时候特殊处理
  {
    lcddev.dir=0;
    lcddev.setxcmd=0X2A;
    lcddev.setycmd=0X2B;
    LCD_SetCursor(0x00,0x0000);		//设置光标位置
    lcddev.dir=1;
    lcddev.setxcmd=0X2B;
    lcddev.setycmd=0X2A;
  }
  else if(lcddev.id==0X9488)
  {
    ILI9488_Lcd_SetCursor(lcddev.setxcmd, lcddev.setycmd, 0x0000, 0x0000, lcddev.width, lcddev.height);
  }
  else if(lcddev.id==SSD1963_ID)
  {
    SSD1963_Lcd_SetCursor(0x0000, 0x0000, lcddev.width-1, lcddev.height-1);
  }
  else LCD_SetCursor(0x00,0x0000);	//设置光标位置
  LCD_WriteRAM_Prepare();     		//开始写入GRAM
  for(index=0; index<totalpoint; index++)
  {
    LCD_WR_DATA(color);
  }
}

/**
 * @brief LCD_SetCursor 设置光标位置
 * @param Xpos 横坐标
 * @param Ypos 纵坐标
 */
void LCD_SetCursor(UINT16 Xpos, UINT16 Ypos)
{
  if(lcddev.id==NT35310_ID)
  {
    NT35310_Lcd_SetCursor(lcddev.setxcmd, lcddev.setycmd, Xpos, Ypos);
  }
  else if(lcddev.id==ILI9488_ID)
  {
    ILI9488_Lcd_SetCursor(lcddev.setxcmd, lcddev.setycmd, Xpos, Ypos, Xpos, Ypos);
  }
  else if(lcddev.id==SSD1963_ID)
  {
    SSD1963_Lcd_SetCursor(Xpos, Ypos, Xpos, Ypos);
  }
}

/**
 * @brief LCD_DrawPoint 画点
 * @param x 坐标
 * @param y
 */
void LCD_DrawPoint(UINT16 x,UINT16 y)
{
  if(lcddev.id == SSD1963_ID)
  {
    Y_1963(y);
  }

  LCD_SetCursor(x,y);		//设置光标位置
  LCD_WriteRAM_Prepare();	//开始写入GRAM
  LCD_WR_DATA(POINT_COLOR);
}

/**
 * @brief LCD_Fast_DrawPoint 快速画点
 * @param x 坐标
 * @param y
 * @param color 颜色
 */
void LCD_Fast_DrawPoint(UINT16 x,UINT16 y,UINT16 color)
{
  //if(lcddev.dir==1)x=lcddev.width-1-x;//横屏其实就是调转x,y坐标
//  LCD_WR_REG(lcddev.setxcmd);
//  LCD_WR_DATA(x>>8);
//  LCD_WR_DATA(x&0XFF);
//  LCD_WR_REG(lcddev.setycmd);
//  LCD_WR_DATA(y>>8);
//  LCD_WR_DATA(y&0XFF);
  LCD_SetCursor(x, y);
  LCD_WriteRAM_Prepare();
  LCD_WR_DATA(color);
}

/**
 * @brief LCD_ReadPoint 读取个某点的颜色值
 * @param x 坐标
 * @param y
 * @return 此点的颜色
 */
UINT16 LCD_ReadPoint(UINT16 x,UINT16 y)
{
  __IO UINT16 r,g=0,b=0;
  if((x>=lcddev.width)||(y>=lcddev.height))return 0;	//超过了范围,直接返回
  LCD_SetCursor(x,y);
  if((lcddev.id==NT35310_ID) || (lcddev.id==ILI9488_ID))
    LCD_WR_REG(0X2E);//发送读GRAM指令

  (void)LCD_RD_DATA();									//dummy Read
  //opt_delay(2);
  r=LCD_RD_DATA();  		  						//实际坐标颜色
  if(lcddev.id==NT35310_ID )	//NT35310要分2次读出
  {
    //opt_delay(2);
    b=LCD_RD_DATA();
    g=r&0XFF;		//对于5310,第一次读取的是RG的值,R在前,G在后,各占8位
    g<<=8;
  }
  else if(lcddev.id==ILI9488_ID)
  {
    b=LCD_RD_DATA();
    g=LCD_RD_DATA();
  }
  else if(lcddev.id==SSD1963_ID)//读一次就可以把整个像素值读出来，要想读多个像素的值需要用0X3E读取MENMORY命令
  {
    LCD_WR_REG(0X2E);//发送读GRAM指令
//		HAL_Delay(1);
    r=LCD_RD_DATA();
    return r;
  }
  return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));//ILI9341/NT35310/NT35510需要公式转换一下
}

/**
 * @brief LCD_Draw_Circle 在指定位置画一个指定大小的圆
 * @param x0 (x,y):中心点
 * @param y0
 * @param r 半径
 */
void LCD_Draw_Circle(UINT16 x0,UINT16 y0,UINT8 r)
{
  //int a,b;
  UINT16 a,b;
  int di;
  a=0;
  b=r;
  di=3-(r<<1);             //判断下个点位置的标志
  while(a<=b)
  {
    LCD_DrawPoint(x0+a,y0-b);             //5
    LCD_DrawPoint(x0+b,y0-a);             //0
    LCD_DrawPoint(x0+b,y0+a);             //4
    LCD_DrawPoint(x0+a,y0+b);             //6
    LCD_DrawPoint(x0-a,y0+b);             //1
    LCD_DrawPoint(x0-b,y0+a);
    LCD_DrawPoint(x0-a,y0-b);             //2
    LCD_DrawPoint(x0-b,y0-a);             //7
    a++;
    //使用Bresenham算法画圆
    if(di<0)di +=(4*a)+6;
    else
    {
      di+=10+(4*(a-b));
      b--;
    }
  }
}

/**
 * @brief LCD_DrawLine 画线
 * @param x1 起点坐标
 * @param y1
 * @param x2 起点坐标
 * @param y2
 */
void LCD_DrawLine(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)
{
  UINT16 t;
  int xerr=0,yerr=0,delta_x,delta_y,distance;
  int incx,incy;
  UINT16 uRow,uCol;
  delta_x=x2-x1; //计算坐标增量
  delta_y=y2-y1;
  uRow=x1;
  uCol=y1;
  if(delta_x>0)incx=1; //设置单步方向
  else if(delta_x==0)incx=0;//垂直线
  else
  {
    incx=-1;
    delta_x=-delta_x;
  }
  if(delta_y>0)incy=1;
  else if(delta_y==0)incy=0;//水平线
  else
  {
    incy=-1;
    delta_y=-delta_y;
  }
  if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴
  else distance=delta_y;
  for(t=0; t<=(distance+1); t++ ) //画线输出
  {
    LCD_DrawPoint(uRow,uCol);//画点
    xerr+=delta_x ;
    yerr+=delta_y ;
    if(xerr>distance)
    {
      xerr-=distance;
      uRow+=(UINT16)incx;
    }
    if(yerr>distance)
    {
      yerr-=distance;
      uCol+=(UINT16)incy;
    }
  }
}

/**
 * @brief LCD_DrawRectangle 画矩形
 * @param x1 矩形的对角坐标
 * @param y1
 * @param x2
 * @param y2
 */
void LCD_DrawRectangle(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)
{
  LCD_DrawLine(x1,y1,x2,y1);
  LCD_DrawLine(x1,y1,x1,y2);
  LCD_DrawLine(x1,y2,x2,y2);
  LCD_DrawLine(x2,y1,x2,y2);
}

/**
 * @brief LCD_Fill 在指定区域内填充单个颜色,特殊地方是在SSD1963驱动的屏幕时，填充特定颜色块
 * @param sx (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
 * @param sy
 * @param ex
 * @param ey
 * @param color 要填充的颜色
 */
void LCD_Fill(UINT16 sx,UINT16 sy,UINT16 ex,UINT16 ey,UINT16 color)
{
  if(lcddev.id==NT35310_ID)
  {
    NT35310_Lcd_Fill(lcddev.setxcmd, lcddev.setycmd, sx, sy, ex, ey, color);
  }
  else if(lcddev.id==ILI9488_ID)
  {
    ILI9488_Lcd_Fill(lcddev.setxcmd, lcddev.setycmd, sx, sy, ex, ey, color);
  }
  else if(lcddev.id == SSD1963_ID)
  {
    ex -= sx;
    X_1963(sx);
    ex += sx;
    ey -= sy;
    Y_1963(sy);
    ey += sy;
    SSD1963_Lcd_Fill( sx, sy, ex, ey, color);
  }
}

void LCD_Fill_Default(UINT16 sx,UINT16 sy,UINT16 ex,UINT16 ey)
{
  LCD_Fill(sx, sy, ex, ey, DEFAULT_DRAW_COLOR);
}

/**
 * @brief LCD_Fill_Picture 只在画图时使用，在指定区域内填充单个颜色
 * @param sx (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
 * @param sy
 * @param ex
 * @param ey
 * @param color 要填充的颜色
 */
void LCD_Fill_Picture(UINT16 sx,UINT16 sy,UINT16 ex,UINT16 ey,UINT16 color)
{
  if(lcddev.id==NT35310_ID)
  {
    NT35310_Lcd_Fill(lcddev.setxcmd, lcddev.setycmd, sx, sy, ex, ey, color);
  }
  else if(lcddev.id==ILI9488_ID)
  {
    ILI9488_Lcd_Fill(lcddev.setxcmd, lcddev.setycmd, sx, sy, ex, ey, color);
  }
  else if(lcddev.id == SSD1963_ID)
  {
    SSD1963_Lcd_Fill( sx, sy, ex, ey, color);
  }
}

/**
 * @brief LCD_Color_Fill 在指定区域内填充指定颜色块
 * @param sx (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
 * @param sy
 * @param ex
 * @param ey
 * @param color 要填充的颜色
 */
void LCD_Color_Fill(UINT16 sx,UINT16 sy,UINT16 ex,UINT16 ey, const UINT16 *color)
{
  if(lcddev.id==NT35310_ID)
  {
    NT35310_Lcd_Color_Fill(lcddev.setxcmd, lcddev.setycmd, sx, sy, ex, ey, color);
  }
  else if(lcddev.id==ILI9488_ID)
  {
    ILI9488_Lcd_Color_Fill(lcddev.setxcmd, lcddev.setycmd, sx, sy, ex, ey, color);
  }
  else if(lcddev.id == SSD1963_ID)
  {
    SSD1963_Lcd_Color_Fill( sx, sy, ex, ey, color);
  }
}

#if USE_LCD_SHOW

/**
 * @brief LCD_ShowChar 在指定位置显示一个字符
 * @param x 起始坐标
 * @param y
 * @param num 要显示的字符:" "--->"~"
 * @param size 字体大小 12/16/24
 * @param mode 叠加方式(1)还是非叠加方式(0)
 */
void LCD_ShowChar(UINT16 x,UINT16 y,UINT8 num,UINT8 size,UINT8 mode)
{
  UINT8 temp,t1,t;
  UINT16 y0=y;
  //得到字体一个字符对应点阵集所占的字节数
  UINT8 csize=((size/8)+((size%8)?1:0))*(size/2);
  //设置窗口
  //得到偏移后的值
  num=num-' ';
  for(t=0; t<csize; t++)
  {
    //调用1206字体
    if(size==12)temp=asc2_1206[num][t];
    //调用1608字体
    else if(size==16)temp=asc2_1608[num][t];
    //调用2412字体
    else if(size==24)temp=asc2_2412[num][t];
    //没有的字库
    else return;
    for(t1=0; t1<8; t1++)
    {
      if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
      else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
      temp<<=1;
      y++;
      //超区域了
      if(y>=lcddev.height)return;
      if((y-y0)==size)
      {
        y=y0;
        x++;
        //超区域了
        if(x>=lcddev.width)return;
        break;
      }
    }
  }
}

/**
 * @brief LCD_ShowNum 显示数字,高位为0,则不显示
 * @param x 起点坐标
 * @param y
 * @param num 数值(0~4294967295)
 * @param len 数字的位数
 * @param size 字体大小
 */
void LCD_ShowNum(UINT16 x,UINT16 y,UINT32 num,UINT8 len,UINT8 size)
{
  UINT8 t,temp;
  UINT8 enshow=0;
  for(t=0; t<len; t++)
  {
    temp=(num/LCD_Pow(10,len-t-1))%10;
    if((enshow==0)&&(t<(len-1)))
    {
      if(temp==0)
      {
        LCD_ShowChar(x+((size/2)*t),y,' ',size,0);
        continue;
      }
      else enshow=1;

    }
    LCD_ShowChar(x+((size/2)*t),y,temp+'0',size,0);
  }
}

/**
 * @brief LCD_ShowxNum 显示数字,高位为0,还是显示
 * @param x 起点坐标
 * @param y
 * @param num 数值(0~999999999)
 * @param len 长度(即要显示的位数)
 * @param size 字体大小
 * @param mode
 * [7]:0,不填充;1,填充0
 * [6:1]:保留
 * [0]:0,非叠加显示;1,叠加显示.
 */
void LCD_ShowxNum(UINT16 x,UINT16 y,UINT32 num,UINT8 len,UINT8 size,UINT8 mode)
{
  UINT8 t,temp;
  UINT8 enshow=0;
  for(t=0; t<len; t++)
  {
    temp=(num/LCD_Pow(10,len-t-1))%10;
    if((enshow==0)&&(t<(len-1)))
    {
      if(temp==0)
      {
        if(mode&0X80)LCD_ShowChar(x+((size/2)*t),y,'0',size,mode&0X01);
        else LCD_ShowChar(x+((size/2)*t),y,' ',size,mode&0X01);
        continue;
      }
      else enshow=1;

    }
    LCD_ShowChar(x+((size/2)*t),y,temp+'0',size,mode&0X01);
  }
}

/**
 * @brief LCD_ShowString 显示字符串
 * @param x 起点坐标
 * @param y
 * @param width 区域大小
 * @param height
 * @param size 字体大小
 * @param p 字符串起始地址
 */
void LCD_ShowString(UINT16 x,UINT16 y,UINT16 width,UINT16 height,UINT8 size,UINT8 *p)
{
  UINT8 x0=x;

  if(lcddev.id == SSD1963_ID)
  {
    X_1963(x);
    Y_1963(y);
  }

  width+=x;
  height+=y;
  while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
  {
    if(x>=width)
    {
      x=x0;
      y+=size;
    }
    if(y>=height)break;//退出
    LCD_ShowChar(x,y,*p,size,0);
    x+=size/2;
    p++;
  }
}
#endif

/**
 * @brief LCD_Scan_Dir 设置LCD的自动扫描方向
 * 注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
 * 所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
 * 9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310/5510等IC已经实际测试
 * dir:0~7,代表8个方向(具体定义见lcd.h)
 * @param dir
 */
void LCD_Scan_Dir(UINT8 dir)
{
//  UINT16 regval=0;
//  UINT16 dirreg=0;
//  UINT16 temp;
  if(lcddev.dir==1)
  {
    switch(dir)//方向转换
    {
    case 0:
      dir=6;
      break;
    case 1:
      dir=7;
      break;
    case 2:
      dir=4;
      break;
    case 3:
      dir=5;
      break;
    case 4:
      dir=1;
      break;
    case 5:
      dir=0;
      break;
    case 6:
      dir=3;
      break;
    case 7:
      dir=2;
      break;
    default:
      break;
    }
  }
  if(lcddev.id==NT35310_ID)
  {
    NT35310_Lcd_Scan_Dir(lcddev.width, lcddev.height, lcddev.setxcmd, lcddev.setycmd, dir);
  }
  else if(lcddev.id==ILI9488_ID)
  {
    ILI9488_Lcd_Scan_Dir(lcddev.width, lcddev.height, lcddev.setxcmd, lcddev.setycmd, dir);
  }
  else if(lcddev.id==SSD1963_ID)
  {
    SSD1963_Lcd_Scan_Dir(lcddev.width, lcddev.height, lcddev.setxcmd, lcddev.setycmd, dir);
  }
}

/**
 * @brief LCD_Display_Dir 设置LCD显示方向
 * @param dir 0,竖屏；1,横屏
 */
void LCD_Display_Dir(UINT8 dir)
{
  lcddev.wramcmd=0X2C;
  lcddev.setxcmd=0X2A;
  lcddev.setycmd=0X2B;

  if(dir==0)			//竖屏
  {
    lcddev.dir=0;	//竖屏
    if((lcddev.id==NT35310_ID) || (lcddev.id==ILI9488_ID))
    {
      lcddev.width=320;
      lcddev.height=480;
    }
    else if(lcddev.id==SSD1963_ID)
    {
      lcddev.width=272;
      lcddev.height=480;
    }
  }
  else 				//横屏
  {
    lcddev.dir=1;	//横屏
    if((lcddev.id==NT35310_ID) || (lcddev.id==ILI9488_ID))
    {
      lcddev.width=480;
      lcddev.height=320;
    }
    else if(lcddev.id==SSD1963_ID)
    {
      lcddev.width=480;
      lcddev.height=272;
    }
  }
  LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}

/**
 * @brief LCD_Set_Window 设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
 * @param sx 窗口起始坐标(左上角)
 * @param sy
 * @param width 窗口宽度和高度,必须大于0!!
 * @param height
 * 68042,横屏时不支持窗口设置!!
 */
void LCD_Set_Window(UINT16 sx,UINT16 sy,UINT16 width,UINT16 height)
{
  UINT8 hsareg,heareg,vsareg,veareg;
  UINT16 hsaval,heaval,vsaval,veaval;
  width=(sx+width)-1;
  height=(sy+height)-1;
  if((lcddev.id==0X9341)||(lcddev.id==NT35310_ID)||(lcddev.id==0X6804))//6804横屏不支持
  {
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(sx>>8);
    LCD_WR_DATA(sx&0XFF);
    LCD_WR_DATA(width>>8);
    LCD_WR_DATA(width&0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(sy>>8);
    LCD_WR_DATA(sy&0XFF);
    LCD_WR_DATA(height>>8);
    LCD_WR_DATA(height&0XFF);
  }
  else if(lcddev.id==0X5510)
  {
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(sx>>8);
    LCD_WR_REG(lcddev.setxcmd+1);
    LCD_WR_DATA(sx&0XFF);
    LCD_WR_REG(lcddev.setxcmd+2);
    LCD_WR_DATA(width>>8);
    LCD_WR_REG(lcddev.setxcmd+3);
    LCD_WR_DATA(width&0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(sy>>8);
    LCD_WR_REG(lcddev.setycmd+1);
    LCD_WR_DATA(sy&0XFF);
    LCD_WR_REG(lcddev.setycmd+2);
    LCD_WR_DATA(height>>8);
    LCD_WR_REG(lcddev.setycmd+3);
    LCD_WR_DATA(height&0XFF);
  }
  else	//其他驱动IC
  {
    if(lcddev.dir==1)//横屏
    {
      //窗口值
      hsaval=sy;
      heaval=height;
      vsaval=lcddev.width-width-1;
      veaval=lcddev.width-sx-1;
    }
    else
    {
      hsaval=sx;
      heaval=width;
      vsaval=sy;
      veaval=height;
    }
    hsareg=0X50;
    heareg=0X51;//水平方向窗口寄存器
    vsareg=0X52;
    veareg=0X53;//垂直方向窗口寄存器
    //设置寄存器值
    LCD_WriteReg(hsareg,hsaval);
    LCD_WriteReg(heareg,heaval);
    LCD_WriteReg(vsareg,vsaval);
    LCD_WriteReg(veareg,veaval);
    LCD_SetCursor(sx,sy);	//设置光标位置
  }
}

///**
// * @brief LCD_BGR2RGB 从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
// * 通过该函数转换
// * @param c GBR格式的颜色值
// * @return RGB格式的颜色值
// */
//static UINT16 LCD_BGR2RGB(UINT16 c)
//{
//  UINT16  r,g,b,rgb;
//  b=c&0x1f;
//  g=(c>>5)&0x3f;
//  r=(c>>11)&0x1f;
//  rgb=(UINT16)((b<<11)+(g<<5)+r);
//  return(rgb);
//}

///**
// * @brief opt_delay 当mdk -O1时间优化时需要设置
// * @param i 延时i
// */
//static void opt_delay(UINT8 i)
//{
//  while(i--);
//}

#if USE_LCD_SHOW
/**
 * @brief LCD_Pow m^n函数
 * @param m
 * @param n
 * @return 返回值:m^n次方.
 */
static UINT32 LCD_Pow(UINT8 m,UINT8 n)
{
  UINT32 result=1;
  while(n--)result*=m;
  return result;
}
#endif

/**
 * @brief LCD_Get_IC_ID
 * @return
 */
static void LCD_Get_IC_ID(void)
{
  HAL_Delay(50); // delay 50 ms
  LCD_WriteReg(0x0000,0x0001);
  HAL_Delay(50); // delay 50 ms
  lcddev.id = LCD_ReadReg(0x0000);
  if((lcddev.id<0XFF)||(lcddev.id==0XFFFF)||(lcddev.id==0X9300))//读到ID不正确,新增lcddev.id==0X9300判断，因为9341在未被复位的情况下会被读成9300
  {
    //尝试9341 ID的读取
    LCD_WR_REG(0XD3);
    lcddev.id=LCD_RD_DATA();	//dummy read
    lcddev.id=LCD_RD_DATA(); 	//读到0X00
    lcddev.id=LCD_RD_DATA();   	//读取93
    lcddev.id<<=8;
    lcddev.id|=LCD_RD_DATA();  	//读取41
    if(lcddev.id!=0X9341)		//非9341,尝试是不是6804
    {
      LCD_WR_REG(0XBF);
      lcddev.id=LCD_RD_DATA();//dummy read
      lcddev.id=LCD_RD_DATA();//读回0X01
      lcddev.id=LCD_RD_DATA();//读回0XD0
      lcddev.id=LCD_RD_DATA();//这里读回0X68
      lcddev.id<<=8;
      lcddev.id|=LCD_RD_DATA();//这里读回0X04
      if(lcddev.id!=0X6804)	//也不是6804,尝试看看是不是NT35310
      {
        lcddev.id = NT35310_Get_IC_ID();
        if(lcddev.id!=NT35310_ID)		//也不是NT35310,尝试看看是不是NT35510
        {
          lcddev.id = ILI9488_Get_IC_ID();
          if(lcddev.id!=ILI9488_ID)
          {
            lcddev.id =SSD1963_Get_IC_ID();
            if(lcddev.id != SSD1963_ID)
            {
              LCD_WR_REG(0XDA00);
              lcddev.id=LCD_RD_DATA();//读回0X00
              LCD_WR_REG(0XDB00);
              lcddev.id=LCD_RD_DATA();//读回0X80
              lcddev.id<<=8;
              LCD_WR_REG(0XDC00);
              lcddev.id|=LCD_RD_DATA();//读回0X00
              if(lcddev.id==0x8000)lcddev.id=0x5510;//NT35510读回的ID是8000H,为方便区分,我们强制设置为5510
            }
          }
        }
      }
    }
  }
}



