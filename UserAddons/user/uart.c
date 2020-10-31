#include "uart.h"
#include "stm32f4xx_hal.h"

//===========================================================================
//===========加入以下代码,支持printf函数,而不需要选择use MicroLIB============
//===========================================================================

extern UART_HandleTypeDef huart1;

#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
  INT handle;
};
FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(INT x)
{
  x = x;
}
void _ttywrch(INT ch)
{
  ch=ch;
}
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE INT __io_putchar(INT ch)
#else
#define PUTCHAR_PROTOTYPE INT fputc(INT ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1 , (uint8_t *)&ch, 1U, 0xFFFF);
  return ch;
}



