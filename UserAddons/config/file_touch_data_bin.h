#ifndef  FILE_TOUCH_DATA_BIN_H
#define  FILE_TOUCH_DATA_BIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void touch_data_write(uint32_t *pBuffer, uint32_t NumToRead);
void touch_data_read(uint32_t *pBuffer, uint32_t NumToRead);


#ifdef __cplusplus
} // extern "C" {
#endif

#endif //FILE_TOUCH_DATA_BIN_H
