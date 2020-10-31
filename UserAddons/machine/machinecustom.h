#ifndef MACHINECUSTOM_H
#define MACHINECUSTOM_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

//extern FLOAT z_max_pos_real_value;

  extern void sysconfig_init(void);

  extern void SaveSelectedModel(void);
  extern void SaveSelectedPicture(void);
  extern void SaveCustomModelId(void);
  extern void SaveSelectedFunction(void);
  extern void SaveCalculateZMaxPos(float ZMaxPosValue);
//  extern void SaveMatCheckAvgVol(CONST FLOAT MatCheckAvgVolValue);
  extern void SaveBedLevelZValue(void);
  extern void SavePidOutputFactorValue(void);
  extern void SaveSelectedlogo(void);
  extern void SaveBezzerSound(void); //保存soundValua；
  extern void SaveZOffsetZero(void) ;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 默认配置
#define DEFAULT_MODEL               0 // 默认配置//0:M14 1:M2030 2:M2041 3:M2048 4:M3145 5:M4141 6:M4040 7:M4141S 8:AMP410W 9:M14R03 10:M2030HY 11:M14S 12:M3145S    //4141S机型的X轴方向不一样
#define DEFAULT_COLORMIXING         0 // 1:打开混色功能 0:关闭混色功能
#define DEFAULT_POWEROFFRECOVERY    0 // 1:打开断电续打功能 0:关闭断电续打功能
#define DEFAULT_MATCHECK            0 // 1:打开断料检测功能 0：关闭断料检测功能
#define DEFAULT_STEP                1 // 1: 电机16细分 2： 电机32细分
#define DEFAULT_NUMAXIS             4 // 4: 基础版4个电机 5：混色版5个电机

// GUI圖片ID
#define PICTURE_IS_CHINESE  1
#define PICTURE_IS_JAPANESE 2
#define PICTURE_IS_ENGLISH  3
#define PICTURE_IS_KOREA    4
#define PICTURE_IS_RUSSIA   5
#define PICTURE_IS_CHINESE_TRADITIONAL   6

#ifdef __cplusplus
} //extern "C"
#endif

#endif // MACHINECUSTOM_H

