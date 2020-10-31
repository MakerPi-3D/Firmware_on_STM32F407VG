#include "file_common.h"
#include "fatfs.h"
#include "user_debug.h"
#include "user_interface.h"

#ifdef __cplusplus
extern "C" {
#endif



#define SGNF (((((('S'<<8)|'G')<<8)|'N')<<8)|'F')

#define BMP_PATH "0:/file2.bmp"       //提取sgcode中的bmp文件后的存放路径20170920

static char bmp_buf[512];

void sgcode_extract_bmp(char *filePathName)
{
  uint32_t RCount;
  MaseHeader header;
  FilesMsg msg;
  FIL *file;
  FIL *file1;
  FRESULT f_res;

  if (strstr(filePathName, ".sgcode") == NULL)
  {
    return;
  }

  file = (FIL *)malloc(sizeof(FIL));
  file1 = (FIL *)malloc(sizeof(FIL));
  USER_EchoLogStr("filePathName=   %s\r\n", filePathName);
  f_open(file, filePathName, FA_READ);

  if (f_size(file) < 100)
  {
    USER_EchoLogStr("File error!!");
  }
  else
  {
    // 读取header
    f_read(file, &header, sizeof(MaseHeader), &RCount);
    // 读取FileMsg
    f_lseek(file, sizeof(MaseHeader) + (0 * sizeof(FilesMsg)));
    f_read(file, &msg, sizeof(FilesMsg), &RCount);
    //将file偏移到bmp文件数据开始处
    f_lseek(file, msg.uFileOfs);
    //创建新文件
    f_res = f_open(file1, BMP_PATH, FA_WRITE | FA_CREATE_ALWAYS);
    USER_EchoLogStr("Create file1= %d\n", f_res);

    if (f_res == FR_NOT_ENOUGH_CORE)
    {
      //      f_mount_Udisk();
      f_close(file1);
      f_close(file);
      free(file);
      free(file1);
      file = NULL;
      file1 = NULL;
      return;
    }

    for (int i = 512; msg.uFileSize > i; msg.uFileSize -= 512)
    {
      f_read(file, bmp_buf, i, &RCount);
      f_lseek(file, msg.uFileOfs += 512);
      f_write(file1, bmp_buf, i, &RCount);
    }

    f_read(file, bmp_buf, msg.uFileSize, &RCount);
    f_lseek(file, msg.uFileOfs += msg.uFileSize);
    f_write(file1, bmp_buf, msg.uFileSize, &RCount);
    f_close(file1);
    USER_EchoLogStr("msg.szFileName=  %s\n", msg.szFileName);
    USER_EchoLogStr("msg.uFileOfs=    %d\n", msg.uFileOfs);
    USER_EchoLogStr("msg.uFileSize=   %d\n", msg.uFileSize);
  }

  f_close(file);
  free(file);
  free(file1);
  file = NULL;
  file1 = NULL;
}

char *sgcode_get_bmp_path(void)
{
  return (char *)BMP_PATH;
}

void sgcode_delete_bmp(void)
{
  (void)f_unlink(BMP_PATH);
}

#ifdef __cplusplus
} // extern "C" {
#endif


