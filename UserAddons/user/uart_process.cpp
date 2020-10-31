#include "uart.h"
#include "cmsis_os.h"
#include "user_debug.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "threed_engine.h"
#include "globalvariables.h"
#include "sysconfig_data.h"
#include "interface.h"
#include "config_model_tables.h"
#include "user_interface.h"
#include "planner.h"
#ifdef __cplusplus
extern "C" {
#endif

//===========================================================================
//=============================private variables ============================
//===========================================================================

  static volatile INT buflen = 0;                             /*!< 环形指令队列数据长度 */
//  static volatile INT bufindr = 0;                            /*!< 环形指令队列尾 */
  volatile bool serial_print[2] = {false};
////  // 打印状态
////  static volatile UCHAR serial_stop_print = 0U;        /*!< 串口停止打印标志位 */
////  static volatile UCHAR serial_pause_print = 0U;       /*!< 串口暂停打印标志位 */
////  static volatile UCHAR serial_resume_print = 0U;      /*!< 串口恢复打印标志位 */
////  static volatile UCHAR is_process_pause = 0U;         /*!< 串口暂停打印执行 */
////  static volatile UCHAR is_process_resume = 0U;        /*!< 串口恢复打印执行 */
////  // 设置喷嘴温度
////  static volatile UCHAR is_set_extruder_temp = 0U;     /*!< 是否设置喷嘴温度 */
////  static volatile UCHAR extruder_active = 0U;          /*!< 喷嘴ID */
////  static volatile UINT extruder_temp_value = 0U;       /*!< 喷嘴温度值 */
////  // 设置热床温度
////  static volatile UCHAR is_set_bed_temp = 0U;          /*!< 是否设置热床温度 */
////  static volatile UCHAR bed_temp_value = 0U;           /*!< 热床温度值 */
////  // 设置速度
////  static volatile UCHAR is_set_speed = 0U;             /*!< 是否设置速度 */
////  static volatile UCHAR speed_value = 0U;              /*!< 速度值 */

////  static volatile UCHAR is_serial_move_xyz = 0U;       /*!< 是否串口移动指令 */
////  static volatile UCHAR is_unlock_motor = 1U;          /*!< 是否解锁电机 */
////  // 反馈温度与位置信息变量
////  static ULONG respond_temp_status_peroid = 0UL;        /*!< 记录温度反馈时间 */
////  static ULONG respond_temp_status_time_count = 0UL;    /*!< 记录温度反馈秒数 */
////  static ULONG respond_pos_status_peroid = 0UL;         /*!< 记录位置反馈时间 */
////  static ULONG respond_pos_status_time_count = 0UL;     /*!< 记录位置反馈秒数 */
////  static UCHAR temp_and_pos_switch = 0U;               /*!< 温度与位置反馈状态切换 */

////  static volatile UCHAR is_homing = 0U;                /*!< G28归零标志 */

  static long gcode_N, gcode_LastN;     //wifi
  static char *strchr_pointer;

  void stepper_quick_stop(void);
  extern void temperature_set_extruder_target(CONST FLOAT celsius, INT extruder);
  extern void temperature_set_bed_target(CONST FLOAT celsius);

  USART_RECEIVETYPE UsartType1;
  extern UART_HandleTypeDef huart1;

  extern void temperature_disable_heater(void);

//===========================================================================
//=============================private function =============================
//===========================================================================

  // 拷贝串口数据
  //static inline void copy_usart_data(void)
  static void copy_usart_data(void)
  {
    for(int i = 0; i < CMD_BUF_SIZE; i++)
    {
      if(i < UsartType1.rx_len)
      {
        command_buffer[command_buffer_head][i] = UsartType1.usartDMA_rxBuf[i];
      }
      else
      {
        command_buffer[command_buffer_head][i] = 0;
      }
    }
    command_buffer[command_buffer_head][UsartType1.rx_len] = 0U;
  }



//===========================================================================
//=============================public  function =============================
//===========================================================================

  //开启串口中断
  void serial_open_int(void)
  {
    HAL_UART_Receive_DMA(&huart1, UsartType1.usartDMA_rxBuf, RECEIVELEN);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  }

  //DMA发送函数
  void Usart1SendData_DMA(uint8_t *pdata, uint16_t Length)
  {
    while(UsartType1.dmaSend_flag == USART_DMA_SENDING);
    UsartType1.dmaSend_flag = USART_DMA_SENDING;
    HAL_UART_Transmit_DMA(&huart1, pdata, Length);
  }

  //DMA发送完成中断回调函数
  void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
  {
    __HAL_DMA_DISABLE(huart->hdmatx);
    UsartType1.dmaSend_flag = USART_DMA_SENDOVER;
  }

  void usart_process(void)
  {

  }

  void buflen_pro(void)
  {
    if(buflen > 0)
      buflen = (buflen-1);
//    bufindr = (bufindr + 1)%BUFSIZE;
  }

  void serial_ClearToSend()
  {
    if(!serial_print[0]) return;
    printf("ok");
    printf("\n");
  }

  void FlushSerialRequestResend()
  {
    printf("Resend: ");
    printf("%ld\n",gcode_LastN + 1);
    serial_ClearToSend();
  }

  void send_serial_command(void)
  {
    sys_send_gcode_cmd_delay(false);
  }

  void get_serial_command(void)
  {
    if(strchr((const char*)command_buffer[command_buffer_head], 'N') != NULL)
    {
      strchr_pointer = strchr(( char*)command_buffer[command_buffer_head], 'N');
      gcode_N = (strtol((const char*)&command_buffer[command_buffer_head][strchr_pointer - command_buffer[command_buffer_head] + 1], NULL, 10));
//			if(gcode_N != gcode_LastN+1 && (strstr(command_buffer[command_buffer_head], PSTR("M110")) == NULL) )
//			{
//				printf("Error:");
//				printf("Line Number is not Last Line Number+1, Last Line: ");
//				printf("%ld",gcode_LastN);
//				FlushSerialRequestResend();
//				return;
//			}

      if(strchr((const char*)command_buffer[command_buffer_head], '*') != NULL)
      {
        UCHAR checksum = 0;
        UCHAR count = 0;
        while(command_buffer[command_buffer_head][count] != '*') checksum = checksum^command_buffer[command_buffer_head][count++];
        strchr_pointer = strchr((char*)command_buffer[command_buffer_head], '*');

        if( (UCHAR)(strtod((const char*)&command_buffer[command_buffer_head][strchr_pointer - command_buffer[command_buffer_head] + 1], NULL)) != checksum)
        {
          printf("Error:");
          printf("checksum mismatch, Last Line: ");
          printf(" checksum: %d\n\r",checksum);
          count = 0;
          printf(" '");
          while(command_buffer[command_buffer_head][count] != '*')
          {
            printf("%c",command_buffer[command_buffer_head][count++]);
          }
          printf(" '\n\r ");
          checksum = 0;
          count = 0;
          while(command_buffer[command_buffer_head][count] != '*')
          {
            printf("command_buffer:%d;",command_buffer[command_buffer_head][count]);
            checksum = checksum^command_buffer[command_buffer_head][count++];
            printf(" checksum:%d \n\r",checksum);
          }
          ///	printf("\n\r ");

          printf("%ld",gcode_LastN);
          FlushSerialRequestResend();
          return;
        }
        //if no errors, continue parsing
      }
      else
      {
        printf("Error:");
        printf("No Checksum with line number, Last Line: ");
        printf("%ld",gcode_LastN);
        FlushSerialRequestResend();
        return;
      }

      gcode_LastN = gcode_N;
			serial_print[1] = true;
      //if no errors, continue parsing
    }
    else  // if we don't receive 'N' but still see '*'
    {
			serial_print[1] = false;
      if((strchr((char*)command_buffer[command_buffer_head], '*') != NULL))
      {
        printf("Error:");
        printf("No Line Number with checksum, Last Line: ");
        printf("%ld",gcode_LastN);
        return;
      }
//      else
//        return;
    }

//		command_buffer_head = (command_buffer_head + 1)%BUFSIZE;
    buflen += 1;//sanse
		serial_print[0] = true;
    send_serial_command();

  }

  //串口接收空闲中断
  void UsartReceive_IDLE(UART_HandleTypeDef *huart)
  {
    uint32_t temp;
    if((__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE) != RESET))
    {
      __HAL_UART_CLEAR_IDLEFLAG(&huart1);
      HAL_UART_DMAStop(&huart1);
      temp = huart1.hdmarx->Instance->NDTR;
      UsartType1.rx_len =  RECEIVELEN - temp;
      UsartType1.receive_flag=1;
      HAL_UART_Receive_DMA(&huart1,UsartType1.usartDMA_rxBuf,RECEIVELEN);
    }
    // 打印过程中，接收到gcode指令后，返回ok申请下一条
    // 直到当前指令条数为BUFSIZE-1，暂停请求
  }

  void process_serial_command(void)
  {
    if(!UsartType1.receive_flag) //如果产生了空闲中断
      return;
    else if(buflen < (BUFSIZE-1))
    {
      copy_usart_data();
      UsartType1.receive_flag = 0;
    }

    if(buflen < (BUFSIZE-1))
      get_serial_command();
  }

#ifdef __cplusplus
} // extern "C" {
#endif

