#ifndef  FILE_COMMON_H
#define  FILE_COMMON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 图片包文件头结构
typedef struct SMaseFileHeader
{
  uint32_t  uFileFlag;          // 包文件头标记: 'MASE'
  uint32_t  uFileCount;         // 包内文件个数
  uint32_t  uFileListOfs;       // 文件列表偏移
  uint32_t  uMaxFileCount;      // 最大子文件个数
  uint32_t  uFileSize;          // 包文件的大小
} MaseHeader;

//包内文件信息结构
typedef struct SFilesMessage
{
  uint32_t  uFileOfs;          // 本文件在包内的偏移
  uint32_t  uFileSize;         // 本文件的大小
  char  szFileName[60];        // 本文件的文件名，包含路径
} FilesMsg;

#ifdef __cplusplus
} // extern "C" {
#endif

#endif //FILE_COMMON_H
