#include "lcd_common.h"
#include "lcd.h"
#ifdef __cplusplus
extern "C" {
#endif

//LCD地址结构体
  typedef struct
  {
    UINT16 LCD_REG;
    UINT16 LCD_RAM;
  } LCD_TypeDef;

//使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 A6作为数据命令区分线
//注意设置时STM32内部会右移一位对其! 111 1110=0X7E
#define LCD_BASE    ((UINT32)(0x60000000 | 0x007ffffe))
#define LCD         ((LCD_TypeDef *) LCD_BASE)
#define LCD_WR_Reg  (LCD->LCD_REG)
#define LCD_WR_Data (LCD->LCD_RAM)

  /**
   * @brief LCD_WR_REG 写寄存器函数
   * @param regval 寄存器值
   */
  void LCD_WR_REG(__IO UINT16 regval)
  {
    HAL_SRAM_Write_16b(&hsram1, (UINT32*)&LCD_WR_Reg, (UINT16 *)&regval, 1);
  }

  /**
   * @brief LCD_RD_REG 读寄存器函数
   * @param regval 寄存器值
   */
  UINT16 LCD_RD_REG(void)
  {
    __IO UINT16 regval;
    HAL_SRAM_Read_16b(&hsram1, (UINT32*)&LCD_WR_Reg, (UINT16 *)&regval, 1);
    return regval;
  }

  /**
   * @brief LCD_WR_DATA 写LCD数据
   * @param data 要写入的值
   */
  void LCD_WR_DATA(__IO UINT16 data)
  {
    HAL_SRAM_Write_16b(&hsram1, (UINT32*)&LCD_WR_Data, (UINT16 *)&data, 1);
  }

  /**
   * @brief LCD_RD_DATA 读LCD数据
   * @return 读到的值
   */
  UINT16 LCD_RD_DATA(void)
  {
    __IO UINT16 ram; //防止被优化
    HAL_SRAM_Read_16b(&hsram1, (UINT32*)&LCD_WR_Data, (UINT16 *)&ram, 1);
    return ram;
  }

  /**
   * @brief LCD_WriteReg 写寄存器
   * @param LCD_Reg 寄存器地址
   * @param LCD_RegValue 要写入的数据
   */
  void LCD_WriteReg(__IO UINT16 LCD_Reg, __IO UINT16 LCD_RegValue)
  {
    LCD_WR_REG(LCD_Reg);
    LCD_WR_DATA(LCD_RegValue);
  }

  /**
   * @brief LCD_ReadReg 读寄存器
   * @param LCD_Reg 寄存器地址
   * @return 读到的数据
   */
  UINT16 LCD_ReadReg(__IO UINT16 LCD_Reg)
  {
    LCD_WR_REG(LCD_Reg);  //写入要读的寄存器序号
    HAL_Delay(1);//delay_us(5);
    return LCD_RD_DATA(); //返回读到的值
  }

  /**
   * @brief LCD_WriteRAM_Prepare 开始写GRAM
   */
  void LCD_WriteRAM_Prepare(void)
  {
    LCD_WR_REG(lcddev.wramcmd);
  }

  /**
   * @brief LCD_WriteRAM LCD写GRAM
   * @param RGB_Code 颜色值
   */
  void LCD_WriteRAM(UINT16 RGB_Code)
  {
    LCD_WR_DATA(RGB_Code);
  }
#ifdef __cplusplus
} //extern "C"
#endif

