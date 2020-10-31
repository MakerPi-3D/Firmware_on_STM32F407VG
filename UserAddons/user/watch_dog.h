#ifndef WATCH_DOG_H
#define WATCH_DOG_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  void iwdg_init(void);         ///< 看门狗初始化
  void iwdg_refresh(void);      ///< 3s喂一次狗，5s没有喂狗会复位重启

#ifdef __cplusplus
} // extern "C" {
#endif

#endif //WATCH_DOG_H

