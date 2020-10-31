#include "machinecustom.h"
#include "sysconfig.h"
#include "globalvariables.h"
#include "user_debug.h"
#include "autobedlevelinterface.h"
#include "ConfigurationStore.h"
#include "config_model_tables.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "temperature.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "Alter.h"
static SysConfigOperation sysConfigOperation;

#ifdef __cplusplus
extern "C" {
#endif

void sysconfig_init(void)
{
  SysConfig sysConfig;
  sysConfig.init();
}

void SaveSelectedModel(void) //保存已选择的机型
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangeModelInfo();
  sysConfigOperation.saveInfo(true);
}

void SaveSelectedPicture(void) //保存已选择的图片
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangePictureInfo();
  sysConfigOperation.saveInfo(true);
}

void SaveCustomModelId(void) //保存已选择的图片
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangeCustomModelId();
  sysConfigOperation.saveInfo(true);
}

void SaveSelectedFunction(void) //保存已选择的功能
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangeFunctionInfo();
  sysConfigOperation.saveInfo(true);
}

void SaveCalculateZMaxPos(FLOAT ZMaxPosValue) //保存测量的Z轴行程
{
  sysConfigOperation.readInfo();

  // 判断校准值，如果大于等于最大Z位置减去CAL_Z_MAX_POS_OFFSET，更新校准值
  // 这里主要更新Z最大点和移动最大点
  // 自动调平高度可能小于默认最大高度，忽略修改
  if (((motion_3d_model.z_max_pos_origin - CAL_Z_MAX_POS_OFFSET) <= ZMaxPosValue) //2017.10.8除去+8;+8是防止校准值小于默认值太多，引起最大行程警报，2017.7.14john
      || (1U == t_sys_data_current.enable_bed_level))
  {
    motion_3d_model.xyz_max_pos[2] = ZMaxPosValue;
    motion_3d_model.xyz_move_max_pos[2] = ZMaxPosValue;
  }
  else
  {
    ZMaxPosValue = motion_3d_model.xyz_max_pos[2];
  }

  t_sys_data_current.poweroff_rec_z_max_value = ZMaxPosValue;
  sysConfigOperation.ChangeZMaxPosValueInfo();
  sysConfigOperation.saveInfo(false);
}

//  void SaveMatCheckAvgVol(CONST FLOAT MatCheckAvgVolValue) //保存测量的断料续打模块空载时的平均电压值
//  {
//    sysConfigOperation.readInfo();
//    t_sys_data_current.material_chk_vol_value = MatCheckAvgVolValue;
//    sysConfigOperation.ChangeMatCheckAvgVolValueInfo();
//    sysConfigOperation.saveInfo(false);
//  }

void SaveBedLevelZValue(void)
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangeBedLevelZAtLFValue();
  sysConfigOperation.ChangeBedLevelZAtRFValue();
  sysConfigOperation.ChangeBedLevelZAtLBValue();
  sysConfigOperation.ChangeBedLevelZAtRBValue();
  sysConfigOperation.ChangeBedLevelZAtMiddleValue();
  sysConfigOperation.saveInfo(false);
}

void SavePidOutputFactorValue(void)
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangePidOutputFactorValue();
  sysConfigOperation.saveInfo(false);
}

void SaveSelectedlogo(void)
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangelogoInfo();
  sysConfigOperation.saveInfo(true);
}

void SaveBezzerSound(void) //保存soundValua；
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangeBezzerSound();
  sysConfigOperation.saveInfo(false);
}

void SaveZOffsetZero(void)
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangeZOffsetZeroValue();
  sysConfigOperation.saveInfo(false);
}

#if LASER_MODE
void SaveLaser(void) //保存soundValua；
{
  sysConfigOperation.readInfo();
  sysConfigOperation.ChangeLaser();
  sysConfigOperation.saveInfo(false);
}
#endif

#ifdef __cplusplus
} //extern "C"
#endif

