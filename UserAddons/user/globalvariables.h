#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#ifdef __cplusplus
extern "C" {
#endif
//#define CAL_Z_ZERO_OFFSET

#include "wbtypes.h"
#include "globalvariables_ccmram.h"
#define OnePageNum 4

  extern char CurrentPath[100];  //当前目录的路径
  extern char DisplayFileName[OnePageNum][100];  //当前GUI要显示的文件名
  extern char SDFileName[50];  //电脑上传的文件名
  extern uint32_t task_schedule_delay_time;
#ifdef ENABLE_AUTOMATIC_REFUELING
  extern bool process_switch_ext;
#endif // #ifdef ENABLE_AUTOMATIC_REFUELING

#ifdef CAL_Z_ZERO_OFFSET
  extern BOOL isCalZero;
//extern FLOAT z_offset_value;
#endif

  typedef struct
  {
    uint8_t IsNotHaveMatInPrint; //在打印的时候是否没料了
    uint8_t IsDisplayDoorOpenInfo; //是否显示门打开的提示信息
    uint8_t IsDoorOpen; //M14R03,M14S检测到门是否打开
    uint8_t doorStatus; //门状态
    uint8_t M600FilamentChangeStatus; //打印中途换料状态
    uint8_t ChangeFilamentHeatStatus; //是否加热完成
    uint8_t IsTransFile;  //是否在上传文件
    uint8_t IsSuccessFilament;  //是否完成进丝、退丝
    uint8_t IsFinishedFilamentHeat;  //是否完成加热-进丝、退丝
    uint8_t IsComputerControlToStopPrint; //电脑端控制停止打印
    uint8_t IsComputerControlToResumePrint; //电脑端控制继续打印
    uint8_t IsComputerControlToPausePrint; //电脑端控制暂停打印
    uint8_t IsFinishedCalculateZPosLimit;  //是否完成了Z轴的行程测量
    uint8_t IsFinishedPowerOffRecoverReady; //是否完成了断电续打的准备
    uint8_t IsWarning;  //错误警告
    uint8_t WarningInfoSelect; //警告信息选择
    uint8_t SDIsInsert;  //SD卡是否插入
    uint8_t IsRootDir;  //当前目录是否是根目录
    uint8_t IsHaveNextPage;  //是否有下一页
    uint8_t CurrentPage;  //当前GUI要显示的页面-当前目录下的文件分成多个GUI页面
    uint8_t cavity_temp_max_value; //腔体最大温度
    uint8_t IsHaveFile[OnePageNum];//是否有需要显示的文件名
    uint8_t IsDir[OnePageNum];  //当前GUI显示的文件是否是目录文件
    uint8_t G28_ENDSTOPS_COMPLETE; //是否归零完成
    uint8_t m109_heating_complete; //喷嘴是否加热完成
    uint8_t m190_heating_complete; //热床是否加热完成
    uint8_t m305_is_force_verify; //是否强制检验gcode
    uint8_t isBeepAlarm; //是否报警蜂鸣
    uint8_t isOpenBeep; //是否打开蜂鸣器
  } T_GUI_P;

  extern volatile T_GUI_P t_gui_p;

  extern void globalvariables_init(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // GLOBALVARIABLES_H

