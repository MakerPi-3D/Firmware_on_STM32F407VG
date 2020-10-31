#include "stm32f4xx.h"
#include "user_interface.h"
#include "user_fatfs.h"
#include "user_debug.h"
#include "user_usbd.h"
#include "watch_dog.h"
#include "misc.h"
#include "uart.h"
#include "cmsis_os.h"
#include "pins.h"
#include <string.h>

#include "globalvariables.h"
#include "config_model_tables.h"

extern osMessageQId GcodeCommandHandle;
extern osSemaphoreId ReadUdiskSemHandle;
extern osSemaphoreId GUISendSemHandle;
extern osSemaphoreId GUIWaitSemHandle;
extern osSemaphoreId ReceiveUartCmdHandle;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;

extern ADC_HandleTypeDef hadc1;
static ADC_ChannelConfTypeDef sConfig;


void main_user_core1_init(void)
{
  //执行升级程序后所需的操作
  SCB->VTOR=NVIC_VectTab_FLASH;
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00020000U); //重设中断向量表	0x8020000 0xE0000     0x8000000  0x100000
  __enable_irq();
}

void main_user_core2_init(void)
{
  // 高速USB bulk传输初始化，用来从电脑传输文件
  usbd_init();

  HAL_ResumeTick();

  // 串口初始化
  serial_open_int();

  // 看门狗初始化
  iwdg_init();
}


bool sys_receive_gcode_cmd(void)
{
  bool result = false;
  osEvent evt = osMessageGet(GcodeCommandHandle, osWaitForever);
  if (evt.status == osEventMessage)
  {
    uint32_t value = evt.value.v;
    if(value < BUFSIZE) //防止数组溢出
    {
      command_buffer_tail = value;
      int buf_len = strlen((const char*)command_buffer[command_buffer_tail]);
      if (buf_len > 0)
      {
        // 修复联机打印第一层没变化，cura生成的gcode多了空格，导致校准异常跳出该指令
        // 软件修复后，可删除该逻辑
        char *pStr = strstr((const char*)command_buffer[command_buffer_tail]," E ");
        if(NULL != pStr)
        {
          strcpy(pStr+2, pStr+3);
          command_buffer[command_buffer_tail][buf_len - 1] = '\0';
        }
        result = true;
      }
    }
  }
  return result;
}

void sys_clear_gcode_cmd(void)
{
  osEvent evt = osMessageGet(GcodeCommandHandle, 0U);
  while (evt.status == osEventMessage)
  {
    evt = osMessageGet(GcodeCommandHandle, 0U);
  }
}

void sys_delay(const int tick_value)
{
  HAL_Delay(tick_value);
}


void sys_time2_start(void)
{
  HAL_TIM_Base_Start_IT(&htim2);
}

void sys_time5_write_fan_pwm(int pwm_value)
{
#if defined(FAN_PIN) && (FAN_PIN_ON > -1)
  if(pwm_value>255)
  {
    pwm_value=255;
  }
  pwm_value=(pwm_value*1000)/255;
  TIM_OC_InitTypeDef sConfigOC;
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = (UINT32)pwm_value;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  (void)HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_2);
  (void)HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_2);
#endif//FAN_PIN > -1
}

void task_read_udisk_wait(void)
{
  osSemaphoreWait(ReadUdiskSemHandle, osWaitForever);	//信号量等待
}

void task_read_udisk_release(void)
{
  osSemaphoreRelease(ReadUdiskSemHandle);
}

void task_gui_wait_release(void)
{
  osSemaphoreRelease(GUIWaitSemHandle);
}

bool task_is_gui_send_wait_done(void)
{
  bool result = osSemaphoreWait(GUISendSemHandle, osWaitForever)==osOK;
  return result;
}

void task_receive_uart_wait(void)
{
  osSemaphoreWait(ReceiveUartCmdHandle, osWaitForever);
}

void task_receive_uart_release(void)
{
  osSemaphoreRelease(ReceiveUartCmdHandle);
}

// 设置混色5v风扇状态
void gpio_color_mix_control(bool on_or_off)
{
  if(t_sys_data_current.enable_color_mixing) //混色打开5V风扇
  {
    if(t_sys_data_current.model_id != M41G) //M41G 5V_FAN做断料检测IO
    {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, on_or_off?GPIO_PIN_SET:GPIO_PIN_RESET);
    }
  }
}

void gpio_light_control(bool is_on)
{
  if(is_on)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
  }
}

void gpio_beep_control(bool is_on)
{
  t_gui_p.isOpenBeep = is_on;
#if defined(BEEPER_PIN) && (BEEPER_PIN_ON > -1)
  if(is_on)
  {
    digitalWrite(BEEPER_PIN, GPIO_PIN_SET);
  }
  else
  {
    digitalWrite(BEEPER_PIN, GPIO_PIN_RESET);
  }
#endif
}

void gpio_e_motor_fan_control(bool is_on)
{
#if defined(MOTOR_FAN_PIN) && (MOTOR_FAN_PIN_ON > -1)
  if(is_on)
  {
    digitalWrite(MOTOR_FAN_PIN, GPIO_PIN_SET);
  }
  else
  {
    digitalWrite(MOTOR_FAN_PIN, GPIO_PIN_RESET);
  }
#endif
}

void gpio_board_fan_control(bool is_on)
{
#if defined(BOARD_FAN_PIN) && (BOARD_FAN_PIN_ON > -1)
  if(is_on)
  {
    digitalWrite(BOARD_FAN_PIN, GPIO_PIN_SET);
  }
  else
  {
    digitalWrite(BOARD_FAN_PIN, GPIO_PIN_RESET);
  }
#endif
}

void gpio_infrared_level_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  // 开启红外检测
  HAL_GPIO_DeInit(E1_STEP_PIN_GPIO, E1_STEP_PIN);
  /*Configure GPIO pin : PE2*/
  GPIO_InitStruct.Pin = E1_STEP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(E1_STEP_PIN_GPIO, &GPIO_InitStruct);
}

bool gpio_infrared_level_detection(void)
{
  return HAL_GPIO_ReadPin(E1_STEP_PIN_GPIO,E1_STEP_PIN);
}

bool gpio_mechanical_level_detection(void)
{
  return HAL_GPIO_ReadPin(TEMP_0_PIN_GPIO,TEMP_0_PIN);
}

//void gpio_motor_fan_control(bool is_on)
//{
//  HAL_GPIO_WritePin(MOTOR_FAN_PIN_GPIO, MOTOR_FAN_PIN, (is_on?GPIO_PIN_SET:GPIO_PIN_RESET));
//}

void gpio_block_detect_init(void)
{
  // 默认PA5注册为断料检测ADC引脚，如果开启堵料检测，需要复位PA5
  if(hadc1.Instance==ADC1)
  {
    __ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
  }

  // 引脚初始化
  GPIO_InitTypeDef GPIO_InitStruct;
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();

  // A相
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

  // B相
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  __ADC1_CLK_ENABLE();
}

bool gpio_block_detect_is_a_detection(void)
{
  const bool result = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET;
  return result;
}

bool gpio_block_detect_is_b_detection(void)
{
  const bool result = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET;
  return result;
}

void gpio_material_check_init(void)
{
  if(hadc1.Instance==ADC1)
  {
    __ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
  }

  // 引脚初始化
  GPIO_InitTypeDef GPIO_InitStruct;
  __GPIOA_CLK_ENABLE();
  // A相
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

  __ADC1_CLK_ENABLE();
}

void gpio_material_chk_adc_start(void)
{
#if defined(MATCheck_PIN) && (MATCheck_PIN_ON > -1)
  sConfig.Channel = MATCheck_PIN_ADC_CHANNEL;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  (void)HAL_ADC_ConfigChannel(&hadc1, &sConfig);
  (void)HAL_ADC_Start(&hadc1);
#endif
}

uint32_t gpio_material_chk_adc_get_value(void)
{
#if defined(MATCheck_PIN) && (MATCheck_PIN_ON > -1)
  return HAL_ADC_GetValue(&hadc1);
#endif
}

void gpio_infrated_bed_level_init(bool isDeInit)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(!isDeInit)
  {
    /*Configure GPIO pin : PE2*/
    GPIO_InitStruct.Pin = E1_STEP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(E1_STEP_PIN_GPIO, &GPIO_InitStruct);

  }
  else
  {
    /*Configure GPIO pin : PE2*/
    GPIO_InitStruct.Pin = E1_STEP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(E1_STEP_PIN_GPIO, &GPIO_InitStruct);
  }
}

bool gpio_is_zmin_pin_exit(void)
{
  return (-1 != Z_MIN_PIN);
}

bool gpio_is_temp0_pin_exit(void)
{
#if defined(TEMP_0_PIN) && TEMP_0_PIN_ON > -1
  return (-1 != TEMP_0_PIN);
#else
  return false;
#endif
}

bool gpio_is_temp1_pin_exit(void)
{
#if defined(TEMP_1_PIN) && TEMP_1_PIN_ON > -1
  return (-1 != TEMP_1_PIN);
#else
  return false;
#endif
}

bool gpio_is_bedtemp_pin_exit(void)
{
#if defined(TEMP_BED_PIN) && TEMP_BED_PIN_ON > -1
  return (-1 != TEMP_BED_PIN);
#else
  return false;
#endif
}











