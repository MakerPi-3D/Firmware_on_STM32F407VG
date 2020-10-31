#ifndef ALTER_H
#define ALTER_H
#include "stdint.h"
#include "wbtypes.h"
#include "stdbool.h"

#define TIME_PER   1000

#define MM_PER_ARC_SEGMENT 1
#define N_ARC_CORRECTION 25
#define LASER_MODE 1

#ifdef __cplusplus
extern "C" {
#endif
  typedef enum
  {
    MODE_MIAN = 0,
    MODE_SET,
    MODE_PREPARE,
    MODE_FILUM,
    MODE_FILUM_LEVEL,
    MODE_SET_LAN,
  }
  MODEValueTypeDef;

  extern MODEValueTypeDef Gui_Mode;
  extern CHAR TempMinErr_cnt;
  extern volatile bool serial_print[2];
  extern bool Zaxis_RunOnce;
  extern UINT8 TextDisplayX;

  void Mat_Cut_Init(void);
  void Fan_5v_Init(void);
  void Set_BL(USHORT per);
  void BL_Pro(void);
  CHAR Check_TempMinErr(void);
  void While_Pro(void);

#if LASER_MODE
  void Laser_change_warn(void);
  void Laser_nonsupport(void);
  void Laser_up(void);
  void Laser_change_sure(void);
#endif


#ifdef __cplusplus
} // extern "C" {
#endif


#endif

