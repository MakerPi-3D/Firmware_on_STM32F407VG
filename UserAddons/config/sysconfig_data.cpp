#include "sysconfig_data.h"

#include "user_interface.h"
#include "fatfs.h"
#include "user_debug.h"
#include "globalvariables.h"
#ifdef __cplusplus
extern "C" {
#endif

static FIL sysInfofile;    /* file objects */
static uint32_t sys_data_size = 0;                        // sysconfig數據緩存大小

void sys_write_info_to_sd(const char *filePath)
{
  uint32_t NewInfofile_wr;    /* File R/W count */

  if (f_open(&sysInfofile, filePath, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)  //打开文件
  {
    (void)f_write(&sysInfofile, sys_data, sys_data_size, &NewInfofile_wr);// 写文件
    (void)f_close(&sysInfofile);                                    // 关闭文件
  }

}

void sys_read_info_from_sd(const char *filePath)
{
  uint32_t Infofile_br;                                                  //已经读写的字节数

  if (f_open(&sysInfofile, (char *)"0:/SysConfig1.txt", FA_READ) == FR_OK)
  {
    memset(sys_data, 0, 512 * sizeof(char));
    (void)f_read(&sysInfofile, sys_data, sizeof(sys_data), &Infofile_br);

    // 文件无数据直接删除，有数据替换文件
    if (0 == Infofile_br)
    {
      f_unlink((char *)"0:/SysConfig1.txt");
      f_close(&sysInfofile);
    }
    else
    {
      f_unlink(filePath);
      f_close(&sysInfofile);
      f_rename((char *)"0:/SysConfig1.txt", filePath);
    }
  }

  if (f_open(&sysInfofile, filePath, FA_READ) == FR_OK)          //打开文件（读）
  {
    /**********文件打开成功**************/
    memset(sys_data, 0, 512 * sizeof(char));
    (void)f_read(&sysInfofile, sys_data, sizeof(sys_data), &Infofile_br); //读取文件，最大512字节
    sys_data_size = Infofile_br;                                          //保存读取到的字节数
    sys_data[sys_data_size] = 0;                                           //在读取到的最后一个字节后面一个地址写0，表示文件结束
    (void)f_close(&sysInfofile);                                        //退出文件
  }
  else
  {
    USER_ErrLog("sysconfig open failed!");                        //文件打开失败，打印错误信息
  }

}

#ifdef __cplusplus
} // extern "C" {
#endif

