#ifndef UART_H
#define UART_H

#include "stm32f4xx_hal.h"
#include "wbtypes.h"
#ifdef __cplusplus
extern "C" {
#endif

  /* USER CODE BEGIN Private defines */
#define RECEIVELEN 96
#define USART_DMA_SENDING 1//发送未完成  
#define USART_DMA_SENDOVER 0//发送完成  
  typedef struct
  {
    UCHAR receive_flag:1;//空闲接收标记
    UCHAR dmaSend_flag:1;//发送完成标记
    USHORT rx_len;//接收长度
    UCHAR usartDMA_rxBuf[RECEIVELEN];//DMA接收缓存
  } USART_RECEIVETYPE;

  extern USART_RECEIVETYPE UsartType1;
  /* USER CODE END Private defines */

  extern void serial_open_int(void);
  extern void process_serial_command(void);
  extern void usart_process(void);
  extern void UsartReceive_IDLE(UART_HandleTypeDef *huart);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif //UART_H

