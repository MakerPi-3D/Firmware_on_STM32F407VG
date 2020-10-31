#ifndef SYSCONFIG_H
#define SYSCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "alter.h"

#ifdef __cplusplus
} //extern "C"
#endif

#define APP2_VERSION ((const char*)"V4.0.0")

class SysConfigOperation
{
public:
  SysConfigOperation();

  void readInfo(void);
  void ChangeBootVersionInfo(void);
  void ChangeModelInfo(void);
  void ChangePictureInfo(void);
  void ChangeFunctionInfo(void);
  void ChangeZMaxPosValueInfo(void);
  void ChangeMatCheckAvgVolValueInfo(void);
  void ChangeBedLevelZAtLFValue(void);
  void ChangeBedLevelZAtRFValue(void);
  void ChangeBedLevelZAtLBValue(void);
  void ChangeBedLevelZAtRBValue(void);
  void ChangeBedLevelZAtMiddleValue(void);
  void ChangePidOutputFactorValue(void);
  void ChangelogoInfo(void);
  void ChangeBezzerSound(void);
  void ChangeZOffsetZeroValue(void);
#if LASER_MODE
  void ChangeLaser(void);
#endif
  void ChangeIsSoftfilament(void);
//  void ChangeLOGOinterface(void);
  void ChangeLogoID(void);
  void ChangeCustomModelId(void);

  void saveInfo(bool isFunction);
private:
  /*save sysconfig info Function*/
  void changeMachineSettingMark(void);
};

class SysConfig
{
public:
  SysConfig();
  void init(void);

private:
  /*get sysconfig info Function*/
  void explainInfo(void);
  char* find_key_value(const char* key);

  void getFWBootVersion(void);
  void getFWApp2Version(void);
  void getGetFWBuildDate(void);
  void getMachineSettingMark(void);
  void getModelId(void);
  void getModelPicId(void);
  void getPowerOffRecoveryMask(void);
  void getPowerOffRecoveryZMaxValue(void);
  void getColorMixingMask(void);
  void getMaterialCheckMask(void);
  void getMaterialCheckVolValue(void);
  void getBlockDetectMask(void);
  void getBedLevelZAtLF(void);
  void getBedLevelZAtRF(void);
  void getBedLevelZAtLB(void);
  void getBedLevelZAtRB(void);
  void getBedLevelZAtMiddle(void);
  void getPidOutputFactor(void);
  void getAutoBedLevelMask(void);
  void getisSoftfilament(void);
  void getLOGOinterface(void);
  void getlogoID(void);
  void getCustomModelId(void);

  void getModelStr(void);
  void getStatusInfoStr(void);
  void getBezzerSound(void);
  void sysconfigLog(void);
  void getZOffsetZeroValue(void);
  void getV5ExtruderMask(void);
  void getCF26(void);
  void getCF27(void);
  void getCF28(void);
  void getCF29(void);
  void getCF30(void);
  void getCF31(void);
  void getCF32(void);
  void getCF33(void);
  void getCF34(void);
  void getCF35(void);
  void getCF36(void);
  void getCF37(void);
  void getCF38(void);
  void getCF39(void);
  void getCF40(void);
};

#endif // SYSCONFIG_H

