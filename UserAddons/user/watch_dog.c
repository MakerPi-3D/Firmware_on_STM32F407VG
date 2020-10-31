#include "watch_dog.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "RespondGUI.h"
#include "user_interface.h"
#define WATCH_DOG_SUPPORT

extern IWDG_HandleTypeDef hiwdg;
static bool iwdg_is_reset(void);     ///< 判断是否是看门狗的复位

void iwdg_init(void)
{
#ifdef WATCH_DOG_SUPPORT
  // 触发看门狗，显示报警页面
  if(iwdg_is_reset())
  {
    PopWarningInfo(IWDGResetWarning);
  }
  //开启独立看门狗
//  HAL_IWDG_Start(&hiwdg);
#endif //WATCH_DOG_SUPPORT
}

//判断是否是看门狗的复位
static bool iwdg_is_reset(void)
{
#ifdef WATCH_DOG_SUPPORT
  if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)!=RESET)
  {
    __HAL_RCC_CLEAR_RESET_FLAGS();
    return true;
  }
#endif //WATCH_DOG_SUPPORT
  return false;
}

//3s喂一次狗，5s没有喂狗会复位重启
void iwdg_refresh(void)
{
#ifdef WATCH_DOG_SUPPORT
  static ULONG RefreshWatchDogTimeOut=0;
  if(RefreshWatchDogTimeOut<sys_task_get_tick_count())
  {
    (void)HAL_IWDG_Refresh(&hiwdg);
    RefreshWatchDogTimeOut=sys_task_get_tick_count()+3000;
  }
#endif //WATCH_DOG_SUPPORT
}

