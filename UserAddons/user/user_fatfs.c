#include "user_fatfs.h"
#include "fatfs.h"
#include "usb_host.h"
#include "globalvariables.h"
#include "user_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

  FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
  FATFS SDCardFatFs;            /* File system object for USB disk logical drive */

  void f_mount_SDCard(void)
  {
    if(f_mount(&SDCardFatFs, (TCHAR CONST*)SDPath, 0) == FR_OK)
    {
      USER_EchoLog("SDCard f_mount ok!");
    }
    else
    {
      USER_EchoLog("SDCard f_mount not ok!");
    }
  }

  void f_mount_Udisk(void)
  {
    if(f_mount(&USBDISKFatFs, (TCHAR CONST*)USBHPath, 0) == FR_OK)
    {
      USER_EchoLog("USB f_mount ok!");
    }
    else
    {
      USER_EchoLog("USB f_mount not ok!");
    }
  }

  void f_unmount_Udisk(void)
  {
    if(f_mount(NULL, (TCHAR CONST*)USBHPath, 0) == FR_OK)
    {
      USER_EchoLog("USB f_unmount ok!");
    }
    else
    {
      USER_EchoLog("USB f_unmount not ok!");
    }
  }

  void UpdateUdiskStatus(void)
  {
    extern ApplicationTypeDef Appli_state;
    static ApplicationTypeDef Last_Appli_state= APPLICATION_IDLE;

    if(Appli_state == APPLICATION_START)
    {
      osDelay(20);
    }

    if((Last_Appli_state == APPLICATION_DISCONNECT || Last_Appli_state == APPLICATION_IDLE) && Appli_state == APPLICATION_READY && 0 == t_gui_p.SDIsInsert)
    {
      if(f_mount(&USBDISKFatFs, (TCHAR const*)USBHPath, 1) == FR_OK)
      {
        Last_Appli_state = APPLICATION_READY;
        t_gui_p.SDIsInsert = 1;
        USER_EchoLog("Udisk Insert!");
      }
    }
    if(Last_Appli_state==APPLICATION_READY && Appli_state==APPLICATION_DISCONNECT && 1 == t_gui_p.SDIsInsert)
    {
      if(f_mount(NULL, (TCHAR const*)USBHPath, 1) == FR_OK)
      {
        Last_Appli_state = APPLICATION_DISCONNECT;
        t_gui_p.SDIsInsert = 0;
        USER_EchoLog("Udisk Remove!");
      }
    }
  }

#ifdef __cplusplus
} //extern "C"
#endif

