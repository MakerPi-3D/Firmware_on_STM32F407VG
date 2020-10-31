#include "interface.h"
#include "cmsis_os.h"
//全局信号量
extern osSemaphoreId GUISendSemHandle;
extern osSemaphoreId GUIWaitSemHandle;

SettingInfo SettingInfoToSYS;

void respond_gui_send_sem(CONST INT sempValue)
{
  SettingInfoToSYS.GUISempValue=sempValue;
  (void)osSemaphoreRelease(GUISendSemHandle);
  (void)osSemaphoreWait(GUIWaitSemHandle, osWaitForever);
}

