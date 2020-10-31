#ifndef  FILE_SGCODE_H
#define  FILE_SGCODE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void sgcode_extract_bmp(char *filePathName);
extern char *sgcode_get_bmp_path(void);
extern void sgcode_delete_bmp(void);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif //FILE_SGCODE_H
