#include "globalvariables.h"

//SD卡信息结构
char CurrentPath[100];                  //当前目录的路径
char DisplayFileName[OnePageNum][100];  //当前GUI要显示的文件名
char SDFileName[50];  //电脑上传的文件名

uint32_t task_schedule_delay_time = 50;

#ifdef CAL_Z_ZERO_OFFSET
BOOL isCalZero = FALSE;
#endif

//状态信息结构
volatile T_GUI_P t_gui_p;

void globalvariables_init(void)
{
  ccmram_data_init();
  t_gui_p.IsNotHaveMatInPrint = 0; //在打印的时候是否没料了
  t_gui_p.IsDisplayDoorOpenInfo = 0; //是否显示门打开的提示信息
  t_gui_p.IsDoorOpen = 0; //M14R03,M14S检测到门是否打开
  t_gui_p.doorStatus = 0; //门状态
  t_gui_p.M600FilamentChangeStatus = 0; //打印中途换料状态
  t_gui_p.ChangeFilamentHeatStatus = 0; //是否加热完成
  t_gui_p.IsTransFile = 0;  //是否在上传文件
  t_gui_p.IsSuccessFilament = 0;  //是否完成进丝、退丝
  t_gui_p.IsFinishedFilamentHeat = 0;  //是否完成加热-进丝、退丝
  t_gui_p.IsComputerControlToStopPrint = 0; //电脑端控制停止打印
  t_gui_p.IsComputerControlToResumePrint = 0; //电脑端控制继续打印
  t_gui_p.IsComputerControlToPausePrint = 0; //电脑端控制暂停打印
  t_gui_p.IsFinishedCalculateZPosLimit = 0;  //是否完成了Z轴的行程测量
  t_gui_p.IsFinishedPowerOffRecoverReady = 0; //是否完成了断电续打的准备
  t_gui_p.IsWarning = 0;  //错误警告
  t_gui_p.WarningInfoSelect = 0; //警告信息选择
  t_gui_p.SDIsInsert = 0;  //SD卡是否插入
  t_gui_p.IsRootDir = 0;  //当前目录是否是根目录
  t_gui_p.IsHaveNextPage = 0;  //是否有下一页
  t_gui_p.CurrentPage = 0;  //当前GUI要显示的页面-当前目录下的文件分成多个GUI页面
  t_gui_p.cavity_temp_max_value = 50; //腔体最大温度

  for(int i = 0; i < OnePageNum; i++)
  {
    t_gui_p.IsHaveFile[i] = 0;
    t_gui_p.IsDir[i] = 0;
  }
  t_gui_p.G28_ENDSTOPS_COMPLETE = 0; //是否归零完成
  t_gui_p.m109_heating_complete = 0; //喷嘴是否加热完成
  t_gui_p.m190_heating_complete = 0; //热床是否加热完成
  t_gui_p.m305_is_force_verify = 0; //是否强制检验gcode
  t_gui_p.isBeepAlarm = 0; //是否报警蜂鸣
  t_gui_p.isOpenBeep = 0; //是否打开蜂鸣器
}


