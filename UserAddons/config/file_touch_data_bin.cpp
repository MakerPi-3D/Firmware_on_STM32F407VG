#include "user_interface.h"
#include "fatfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TouchDataFile "0:/TouchData.bin"
#define TouchDataFile43 "0:/TouchData43.bin"

static FIL SaveFile;
void touch_data_write(const uint32_t *pBufferWrite, uint32_t NumToWrite)
{
  uint32_t i;
  uint8_t DataBuf[24] = {0};
  char fileName[20];
  memset(fileName, 0, sizeof(fileName));

  if (t_sys_data_current.ui_number == 1 || t_sys_data_current.ui_number == 2)
  {
    strncpy(fileName, TouchDataFile43, strlen(TouchDataFile43));
    fileName[strlen(TouchDataFile43)] = '\0';
  }
  else
  {
    strncpy(fileName, TouchDataFile, strlen(TouchDataFile));
    fileName[strlen(TouchDataFile)] = '\0';
  }

  for (i = 0; i < NumToWrite; ++i)
  {
    DataBuf[i * 4] = (uint8_t)pBufferWrite[i];
    DataBuf[(i * 4) + 1] = (uint8_t)(pBufferWrite[i] >> 8);
    DataBuf[(i * 4) + 2] = (uint8_t)(pBufferWrite[i] >> 16);
    DataBuf[(i * 4) + 3] = (uint8_t)(pBufferWrite[i] >> 24);
  }


  if (f_open(&SaveFile, fileName, FA_WRITE) == FR_OK)
  {
    (void)f_write(&SaveFile, DataBuf, NumToWrite * 4, (uint32_t *)NULL); //写入24字节
    (void)f_close(&SaveFile);
  }

}

void touch_data_read(uint32_t *pBufferRead, uint32_t NumToRead)
{
  uint32_t i;
  uint8_t DataBuf[24] = {0};
  char fileName[20];
  memset(fileName, 0, sizeof(fileName));

  if (t_sys_data_current.ui_number == 1 || t_sys_data_current.ui_number == 2)
  {
    strncpy(fileName, TouchDataFile43, strlen(TouchDataFile43));
    fileName[strlen(TouchDataFile43)] = '\0';
  }
  else
  {
    strncpy(fileName, TouchDataFile, strlen(TouchDataFile));
    fileName[strlen(TouchDataFile)] = '\0';
  }

  if (f_open(&SaveFile, fileName, FA_READ) == FR_OK)
  {
    (void)f_read(&SaveFile, DataBuf, NumToRead * 4, (uint32_t *)NULL); //读取24字节
    (void)f_close(&SaveFile);
  }


  for (i = 0; i < NumToRead; ++i)
  {
    pBufferRead[i] = (uint32_t)DataBuf[i * 4];
    pBufferRead[i] += (((uint32_t)DataBuf[(i * 4) + 1]) << 8);
    pBufferRead[i] += (((uint32_t)DataBuf[(i * 4) + 2]) << 16);
    pBufferRead[i] += (((uint32_t)DataBuf[(i * 4) + 3]) << 24);
  }
}

#ifdef __cplusplus
} // extern "C" {
#endif


