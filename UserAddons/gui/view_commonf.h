#ifndef VIEW_COMMONF_H
#define VIEW_COMMONF_H

namespace gui_view
{
#ifdef __cplusplus
  extern "C" {
#endif

  extern void CalculatingZMaxLimit(void);
  extern void NoUdiskF(void);

#ifdef __cplusplus
} //extern "C" {
#endif
}


#ifdef __cplusplus
extern "C" {
#endif

extern void page_homing(void); //归零页面
extern void goto_page_homing(void); //跳转归零页面
extern void maindisplayF(void);
extern void PictureSetting(void);
extern void stopprintF(void);
extern void PausePrintF(void);
extern void ResumePrintF(void);
extern void filescanF(void);
extern void transfileF(void);

extern void prepareF(void);
extern void PrintSet_M14(void);
extern void PrintSet_NotM14_Left(void);
extern void PrintSet_NotM14_Right(void);
extern void PrintSet_Cavity(void);
extern void SetLoadFilamentNozzleTemp(void);
extern void SetUnLoadFilamentNozzleTemp(void);
extern void SetPowerOffRecoverNozzleTemp(void);
extern void SetPowerOffRecoverHotBedTemp(void);
extern void MoveXYZ(void);
extern void ChangeFilamentStatus(void);
extern void ChangeFilamentConfirm(void);
extern void NotHaveMatToChangeFilament(void);
extern void MachineSetting(void);
extern void PowerOffRecover(void);
extern void PowerOffRecoverReady(void);

extern void DoorOpenWarning_StartPrint(void);
extern void DoorOpenWarningInfo_NotPrinting(void);
extern void DoorOpenWarningInfo_Printing(void);
extern void NotHaveMatControlInterface2(void);
extern void NotHaveMatControlInterface1(void);
extern void NoHaveMatWaringInterface(void);
extern void printconfirmF(void);
extern void settingF(void);
extern void statusF(void);
extern void printfinishF(void);
extern void WarningInfoF(void);
extern void DisableStepInfo(void);

extern void loadfilament0F(void);
extern void loadfilament1F(void);
extern void loadfilament2F(void);

extern void unloadfilament0F(void);
extern void unloadfilament1F(void);
extern void unloadfilament2F(void);

extern void ConfirmKey_NotM14_Left(void);
extern void ConfirmKey_NotM14_Right(void);
extern void ConfirmKey_Cavity(void);
extern void CancelKey(void);

extern void FunctionSetting(void);

#ifdef __cplusplus
} // extern "C" {
#endif


#endif // VIEW_COMMONF_H


