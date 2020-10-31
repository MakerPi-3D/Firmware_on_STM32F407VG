#include "user_usbd.h"
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"

__ALIGN_BEGIN_OLD USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

void usbd_init(void)
{
  //高速USB bulk传输初始化，用来从电脑传输文件
  USBD_Init(&USB_OTG_dev,
            USB_OTG_HS_CORE_ID,
            &USR_desc,
            &USBD_CDC_cb,
            &USR_cb);
}



