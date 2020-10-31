#include "lcd_ili9488.h"
#include "lcd_common.h"
//#include "user_debug.h"
#ifdef __cplusplus
extern "C" {
#endif

  void ILI9488_Lcd_Scan_Dir(UINT16 width, UINT16 height, UINT16 Xcmd, UINT16 Ycmd, UINT8 dir)
  {
    UINT16 regval=0;
    UINT16 dirreg=0;

    switch(dir)
    {
    case L2R_U2D://从左到右,从上到下
      regval|=(0<<7)|(0<<6)|(0<<5);
      break;
    case L2R_D2U://从左到右,从下到上
      regval|=(1<<7)|(0<<6)|(0<<5);
      break;
    case R2L_U2D://从右到左,从上到下
      regval|=(0<<7)|(1<<6)|(0<<5);
      break;
    case R2L_D2U://从右到左,从下到上
      regval|=(1<<7)|(1<<6)|(0<<5);
      break;
    case U2D_L2R://从上到下,从左到右
      regval|=(0<<7)|(0<<6)|(1<<5);
      break;
    case U2D_R2L://从上到下,从右到左
      regval|=(0<<7)|(1<<6)|(1<<5);
      break;
    case D2U_L2R://从下到上,从左到右
      regval|=(1<<7)|(0<<6)|(1<<5);
      break;
    case D2U_R2L://从下到上,从右到左
      regval|=(1<<7)|(1<<6)|(1<<5);
      break;
    }

    dirreg=0X36;
    regval|=0X08;//5310/5510/1963不需要BGR
    LCD_WR_REG(Xcmd);
    LCD_WR_DATA(0);
    LCD_WR_DATA(0);
    LCD_WR_DATA((width-1)>>8);
    LCD_WR_DATA((width-1)&0XFF);
    LCD_WR_REG(Ycmd);
    LCD_WR_DATA(0);
    LCD_WR_DATA(0);
    LCD_WR_DATA((height-1)>>8);
    LCD_WR_DATA((height-1)&0XFF);

    regval|=1<<12;
    LCD_WriteReg(dirreg,regval);
    //USER_EchoLogStr("dirreg=%d\r\n",LCD_ReadReg(dirreg));
  }

  void ILI9488_Lcd_Fill(UINT16 Xcmd, UINT16 Ycmd, UINT16 sx, UINT16 sy, UINT16 ex, UINT16 ey, UINT16 color)
  {
    LCD_WR_REG(Xcmd);
    LCD_WR_DATA(sx>>8);
    LCD_WR_DATA(sx&0xff);
    LCD_WR_DATA(ex>>8);
    LCD_WR_DATA(ex&0xff);

    LCD_WR_REG(Ycmd);
    LCD_WR_DATA(sy>>8);
    LCD_WR_DATA(sy&0xff);
    LCD_WR_DATA(ey>>8);
    LCD_WR_DATA(ey&0xff);

    LCD_WriteRAM_Prepare();
    for(int i = 0; i <= (((ey - sy) + 1)*( (ex - sx) + 1)); i++ )
      LCD_WR_DATA(color);
  }

  void ILI9488_Lcd_Color_Fill(UINT16 Xcmd, UINT16 Ycmd, UINT16 sx, UINT16 sy, UINT16 ex, UINT16 ey, const UINT16* color)
  {
    LCD_WR_REG(Xcmd);
    LCD_WR_DATA(sx>>8);
    LCD_WR_DATA(sx&0xff);
    LCD_WR_DATA(ex>>8);
    LCD_WR_DATA(ex&0xff);

    LCD_WR_REG(Ycmd);
    LCD_WR_DATA(sy>>8);
    LCD_WR_DATA(sy&0xff);
    LCD_WR_DATA(ey>>8);
    LCD_WR_DATA(ey&0xff);

    LCD_WriteRAM_Prepare();
    for(int i = 0; i <= (((ey - sy) + 1)*( (ex - sx) + 1)); i++ )
      LCD_WR_DATA(color[i]);
  }

  void ILI9488_Lcd_SetCursor(UINT16 Xcmd, UINT16 Ycmd, UINT16 Xpos, UINT16 Ypos, UINT16 width, UINT16 height)
  {
    LCD_WR_REG(Xcmd);
    LCD_WR_DATA(Xpos>>8);
    LCD_WR_DATA(Xpos&0XFF);
    LCD_WR_DATA(width>>8);
    LCD_WR_DATA(width&0XFF);

    LCD_WR_REG(Ycmd);
    LCD_WR_DATA(Ypos>>8);
    LCD_WR_DATA(Ypos&0XFF);
    LCD_WR_DATA(height>>8);
    LCD_WR_DATA(height&0XFF);
  }

  UINT16 ILI9488_Get_IC_ID(void)
  {
    UINT16 ic_id = 0;
    LCD_WR_REG(0XD3);
    (void)LCD_RD_DATA();
    (void)LCD_RD_DATA();
    ic_id=LCD_RD_DATA();
    ic_id <<= 8;
    ic_id|=LCD_RD_DATA();
    return ic_id;
  }

  void ILI9488_Fsmc_ReInit(void)
  {
    //寄存器清零
    //bank1有NE1~4,每一个有一个BCR+TCR，所以总共八个寄存器。
    //这里我们使用NE1 ，也就对应BTCR[0],[1]。
    hsram1.Instance->BTCR[6]=0X00000000;
    hsram1.Instance->BTCR[7]=0X00000000;
    hsram1.Extended->BWTR[6]=0X00000000;
    //操作BCR寄存器	使用异步模式
    hsram1.Instance->BTCR[6]|=1<<12;		//存储器写使能
    hsram1.Instance->BTCR[6]|=1<<14;		//读写使用不同的时序
    hsram1.Instance->BTCR[6]|=1<<4; 		//存储器数据宽度为16bit
    //操作BTR寄存器
    //读时序控制寄存器
    hsram1.Instance->BTCR[7]|=0<<28;		//模式A
    hsram1.Instance->BTCR[7]|=0XF<<0; 	//地址建立时间(ADDSET)为15个HCLK 1/168M=6ns*15=90ns
    //因为液晶驱动IC的读数据的时候，速度不能太快,尤其是个别奇葩芯片。
    hsram1.Instance->BTCR[7]|=60<<8;  	//数据保存时间(DATAST)为60个HCLK	=6*60=360ns
    //写时序控制寄存器
    hsram1.Extended->BWTR[6]|=0<<28; 	//模式A
    hsram1.Extended->BWTR[6]|=9<<0;		//地址建立时间(ADDSET)为9个HCLK=54ns
    //9个HCLK（HCLK=168M）,某些液晶驱动IC的写信号脉宽，最少也得50ns。
    hsram1.Extended->BWTR[6]|=8<<8; 	//数据保存时间(DATAST)为6ns*9个HCLK=54ns
    //使能BANK1,区域4
    hsram1.Instance->BTCR[6]|=1<<0;		//使能BANK1，区域1
  }

  void ILI9488_Lcd_Init(void)
  {
    /* PGAMCTRL (Positive Gamma Control) (E0h) */
    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x0B);
    LCD_WR_DATA(0x40);
    LCD_WR_DATA(0x8A);
    LCD_WR_DATA(0x4B);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0x0D);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x15);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x0F);

    /* NGAMCTRL (Negative Gamma Control) (E1h)  */
    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x1A);
    LCD_WR_DATA(0x1B);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x0D);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x30);
    LCD_WR_DATA(0x35);
    LCD_WR_DATA(0x43);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x32);
    LCD_WR_DATA(0x36);
    LCD_WR_DATA(0x0F);

    /* Power Control 1 (C0h)  */
    LCD_WR_REG(0xC0);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x15);

    /* Power Control 2 (C1h)  */
    LCD_WR_REG(0xC1);
    LCD_WR_DATA(0x41);

    /* VCOM Control (C5h)  */
    LCD_WR_REG(0xC5);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x0A);// VCOM
    LCD_WR_DATA(0x80);

    /* Memory Access Control (36h)  */
    LCD_WR_REG(0x36);
    LCD_WR_DATA(0x48);

    /* Interface Pixel Format (3Ah) */
    LCD_WR_REG(0x3A);  //Interface Mode Control
    LCD_WR_DATA(0x55);

    LCD_WR_REG(0xB0);  //Interface Mode Control
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xB1);
    LCD_WR_DATA(0xA0);//Frame rate 60HZ

    /* Display Inversion Control (B4h) */
    LCD_WR_REG(0xB4);
    LCD_WR_DATA(0x02);

    /* Display Function Control (B6h)  */
    LCD_WR_REG(0xB6);  //RGB/MCU Interface Control
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x22);

    /* Set Image Function (E9h)  */
    LCD_WR_REG(0xE9);
    LCD_WR_DATA(0x00);

    /* Adjust Control 3 (F7h)  */
    LCD_WR_REG(0XF7);
    LCD_WR_DATA(0xA9);
    LCD_WR_DATA(0x51);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x82);

    /* Sleep Out (11h) */
    LCD_WR_REG(0x11);
    HAL_Delay(150);

    /* Display ON (29h) */
    LCD_WR_REG(0x29);
  }

#ifdef __cplusplus
} //extern "C"
#endif



