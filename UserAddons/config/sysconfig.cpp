#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysconfig.h"
#include "sysconfig_data.h"
#include "user_debug.h"
#include "config_model_tables.h"
#include "Alter.h"
#include "globalvariables.h"
#ifdef __cplusplus
extern "C" {
#endif
UINT8 TextDisplayX;
// ----------创建SysConfig.txt----------
// SysConfig.txt存储的信息不能超出512字节，因为读取的时候只读取512字节
// CF0:V1.0 //Boorloader版本
// CF1:V3.0.0 //当前版本
// CF2:20160405  //日期
// CF3:0     //是否已配置机器 1:YES-已设置过机器，不显示配置界面 0:NO-还没设置过机器，显示配置界面
// CF4:0     //机型 0:M14 1:M2030 2:M2041 3:M2048 4:M3145 5:M4141 6:M4040 7:M4141S 8:AMP410W 9:M14R03 10:M2030HY 11:M14S 12:M3145S
// CF5:1     //界面 1:中文图片  2：日文图片
// CF6:0     //断电续打功能 1:打开  0:关闭
// CF7:0     //Z轴最大位置
// CF8:0     //混色功能 1:打开  0:关闭
// CF9:0     //断料检测功能 1:打开  0:关闭
// CF10:0.5  //断料检测模块电压校准值,空载默认值设为0.5V
// CF11:0    //堵料检测功能 1:打开  0:关闭
// CF12:0    //自动调平 Z在xLeft  ，yFront   高度
// CF13:0    //自动调平 Z在xRight ，yFront   高度
// CF14:0    //自动调平 Z在xLeft  ，yBack    高度
// CF15:0    //自动调平 Z在xRight ，yBack    高度
// CF16:0    //自动调平 Z在xMiddle，yMiddle  高度
// CF17:0    //温度调节系数
// CF18:0    //开启自动调平 1:打开  0:关闭
// CF19:0    //开启软硬料   1:打开  0:关闭
// CF20:0    //开启logo     1:打开  0:关闭
// CF21:0    //Logo id
// CF22:0    //定制机型

static const char *sys_config_file_path = "0:/SysConfig.txt";
static char boot_ver_str[20];                   // boot版本
static char app2_ver_str[20];                   // app2版本
static char build_date_str[20];                 // 制作时间

//#define SYS_CONFIG_FILE                          ((char *)"0:/SysConfig.txt")
#define SYS_CONFIG_CF00_BOOT_VERSION                  ((char *)"CF0:")
#define SYS_CONFIG_CF01_APP2_VERSION                  ((char *)"CF1:")
#define SYS_CONFIG_CF02_DATE                          ((char *)"CF2:")
#define SYS_CONFIG_CF03_MACHINE_SETTING_MASK          ((char *)"CF3:")
#define SYS_CONFIG_CF04_MODEL_ID                      ((char *)"CF4:")
#define SYS_CONFIG_CF05_PICTURE_ID                    ((char *)"CF5:")
#define SYS_CONFIG_CF06_POWEROFF_RECOVERY_MASK        ((char *)"CF6:")
#define SYS_CONFIG_CF07_Z_MAX_POS_VALUE               ((char *)"CF7:")
#define SYS_CONFIG_CF08_COLOR_MIXING_MASK             ((char *)"CF8:")
#define SYS_CONFIG_CF09_MAT_CHECK_MASK                ((char *)"CF9:")
#define SYS_CONFIG_CF10_MAT_CHECK_VOL_VALUE           ((char *)"CF10:")
#define SYS_CONFIG_CF11_BLOCK_DETECT_MASK             ((char *)"CF11:")
#define SYS_CONFIG_CF12_AUTO_BED_LEVEL_Z_LF           ((char *)"CF12:")
#define SYS_CONFIG_CF13_AUTO_BED_LEVEL_Z_RF           ((char *)"CF13:")
#define SYS_CONFIG_CF14_AUTO_BED_LEVEL_Z_LB           ((char *)"CF14:")
#define SYS_CONFIG_CF15_AUTO_BED_LEVEL_Z_RB           ((char *)"CF15:")
#define SYS_CONFIG_CF16_AUTO_BED_LEVEL_Z_MIDDLE       ((char *)"CF16:")
#define SYS_CONFIG_CF17_PID_OUTPUT_FACTOR             ((char *)"CF17:")
#define SYS_CONFIG_CF18_AUTO_BED_LEVEL_MASK           ((char *)"CF18:")
#define SYS_CONFIG_CF19_IS_SOFTFILAMENT               ((char *)"CF19:")
#define SYS_CONFIG_CF20_LOGOorNO                      ((char *)"CF20:")
#define SYS_CONFIG_CF21_LOGOID                        ((char *)"CF21:")
#define SYS_CONFIG_CF22_CUSTOM_MODEL_ID               ((char *)"CF22:")
#define SYS_CONFIG_CF23_CUSTOM_BEZZERSOUND            ((char *)"CF23:")
#define SYS_CONFIG_CF24_Z_OFFSET_ZERO_VALUE           ((char *)"CF24:")
#define SYS_CONFIG_CF25_V5_EXTRUDER_MASK              ((char *)"CF25:")
#define SYS_CONFIG_CF26_ENABLE_CAVITY_TEMP            ((char *)"CF26:")
#define SYS_CONFIG_CF27_TYPE_OF_THERMISTOR            ((char *)"CF27:")
#define SYS_CONFIG_CF28_ENABLE_HIGH_TEMP              ((char *)"CF28:")
#define SYS_CONFIG_CF29_UI_NUMBER                     ((char *)"CF29:")
#define SYS_CONFIG_CF30_IS_2GT                        ((char *)"CF30:")
#define SYS_CONFIG_CF31_IS_MECHANISM_LEVEL            ((char *)"CF31:")
#define SYS_CONFIG_CF32_IS_LASER                      ((char *)"CF32:")
#define SYS_CONFIG_CF33_IS_LASER_MODE                 ((char *)"CF33:")
#define SYS_CONFIG_CF34              ((char *)"CF34:")
#define SYS_CONFIG_CF35              ((char *)"CF35:")
#define SYS_CONFIG_CF36              ((char *)"CF36:")
#define SYS_CONFIG_CF37              ((char *)"CF37:")
#define SYS_CONFIG_CF38              ((char *)"CF38:")
#define SYS_CONFIG_CF39              ((char *)"CF39:")
#define SYS_CONFIG_CF40              ((char *)"CF40:")

//  static bool is_sys_data_add = false;    /* 是否追加sys数据标志 */

// sys_data追加数据
//  static void sys_add_data(const char* key, const char* value, bool isSerial = false)
//  {
//    strcat((char *)sys_data, "\r\n");
//    strcat((char *)sys_data, key);
//    strcat((char *)sys_data, value);
//  }

#ifdef __cplusplus
} //extern "C"
#endif

SysConfigOperation::SysConfigOperation() {}

void SysConfigOperation::readInfo(void)
{
  sys_read_info_from_sd(sys_config_file_path);                                                  //从SD卡读取配置信息
}

void SysConfigOperation::ChangeBootVersionInfo(void)
{
  char *Ptr;
  char BootVersionValueBuf1[15];
  char BootVersionValueBuf2[15];//机型字符串存储空间  共10字节

  //机型字符串存储空间初始化为空白符
  for (int i = 0; i < 15; i++)
  {
    BootVersionValueBuf2[i] = ' ';
  }

  (void)snprintf(BootVersionValueBuf1, sizeof(BootVersionValueBuf1), "%s%s", SYS_CONFIG_CF00_BOOT_VERSION, t_sys.version_str);
  (void)memcpy(BootVersionValueBuf2, BootVersionValueBuf1, strlen(BootVersionValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF00_BOOT_VERSION); //CF4:0,0表示M14
  (void)memcpy(Ptr, BootVersionValueBuf2, sizeof(BootVersionValueBuf2)); //覆盖原来的10个字节
}

/********************************************************************
* @brief   :
* @param   : isFunction ：
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 PM
* @versions：V1.0
********************************************************************/
void SysConfigOperation::saveInfo(bool isFunction)
{
  if (isFunction)
    changeMachineSettingMark();                            // 设备是否已经配置的标志位在 InfoBuf里的位置 置一

  sys_write_info_to_sd(sys_config_file_path);                                         // 将修改后的  InfoBuf 数组数据写进SD卡
}

/********************************************************************
* @brief   : 设置设备是否已经配置的标志位
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 PM
* @versions：V1.0
********************************************************************/
void SysConfigOperation::changeMachineSettingMark(void)
{
  char *Ptr;
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF03_MACHINE_SETTING_MASK); //CF3:0           1--YES   0--NO
  Ptr = Ptr + strlen(SYS_CONFIG_CF03_MACHINE_SETTING_MASK);               //计算设备是否已经配置情况在数组InfoBuf里的位置
  *Ptr = '1'; //给CF3:后面的数据置一，表示已配置机器，重新上电不再显示配置界面
}

/********************************************************************
* @brief   :
* @param   :无
* @retval  :
* @notice  ：
* @author  ：
* @date    ：
* @versions：
********************************************************************/
void SysConfigOperation::ChangeModelInfo(void)
{
  char *Ptr = 0;
  char ModelValueBuf1[10];
  char ModelValueBuf2[10];//机型字符串存储空间  共10字节

  //机型字符串存储空间初始化为空白符
  for (int i = 0; i < 10; i++)
  {
    ModelValueBuf2[i] = ' ';
  }

  (void)snprintf(ModelValueBuf1, sizeof(ModelValueBuf1), "%s%d", SYS_CONFIG_CF04_MODEL_ID, t_sys_data_current.model_id);
  (void)memcpy(ModelValueBuf2, ModelValueBuf1, strlen(ModelValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF04_MODEL_ID); //CF4:0,0表示M14
  (void)memcpy(Ptr, ModelValueBuf2, sizeof(ModelValueBuf2)); //覆盖原来的10个字节
}

/********************************************************************
* @brief   :
* @param   :无
* @retval  :
* @notice  ：
* @author  ：
* @date    ：
* @versions：
********************************************************************/
void SysConfigOperation::ChangePictureInfo(void)
{
  char *Ptr;
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF05_PICTURE_ID); //CF4:1,1为中文图片 2为日文图片
  Ptr = Ptr + strlen(SYS_CONFIG_CF05_PICTURE_ID);
  *Ptr = (char)t_sys_data_current.pic_id + '0';
}

/********************************************************************
* @brief   :
* @param   :无
* @retval  :
* @notice  ：
* @author  ：
* @date    ：
* @versions：
********************************************************************/
void SysConfigOperation::ChangeFunctionInfo(void)
{
  char *Ptr;
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF08_COLOR_MIXING_MASK); //CF5:1,1为打开混色功能 2为关闭混色功能
  Ptr = Ptr + strlen(SYS_CONFIG_CF08_COLOR_MIXING_MASK);
  *Ptr = (char)t_sys_data_current.enable_color_mixing + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF06_POWEROFF_RECOVERY_MASK); //CF6:1,1为打开断电续打功能 2为关闭断电续打功能
  Ptr = Ptr + strlen(SYS_CONFIG_CF06_POWEROFF_RECOVERY_MASK);
  *Ptr = (char)t_sys_data_current.enable_powerOff_recovery + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF09_MAT_CHECK_MASK); //CF9:1,1为打开断料检测功能 2为关闭断料检测功能
  Ptr = Ptr + strlen(SYS_CONFIG_CF09_MAT_CHECK_MASK);
  *Ptr = (char)t_sys_data_current.enable_material_check + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF11_BLOCK_DETECT_MASK); //CF11:1,1为打开堵料检测功能 2为关闭堵料检测功能
  Ptr = Ptr + strlen(SYS_CONFIG_CF11_BLOCK_DETECT_MASK);
  *Ptr = (char)t_sys_data_current.enable_block_detect + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF18_AUTO_BED_LEVEL_MASK); //CF18:1,1为打开自动调平功能 2为关闭堵料检测功能
  Ptr = Ptr + strlen(SYS_CONFIG_CF18_AUTO_BED_LEVEL_MASK);
  *Ptr = (char)t_sys_data_current.enable_bed_level + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF19_IS_SOFTFILAMENT); //CF19:1,1为使用了软料 0为使用默认材料
  Ptr = Ptr + strlen(SYS_CONFIG_CF19_IS_SOFTFILAMENT);
  *Ptr = (char)t_sys_data_current.enable_soft_filament + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF25_V5_EXTRUDER_MASK); //CF19:1,1为使用了软料 0为使用默认材料
  Ptr = Ptr + strlen(SYS_CONFIG_CF25_V5_EXTRUDER_MASK);
  *Ptr = (char)t_sys_data_current.enable_v5_extruder + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF26_ENABLE_CAVITY_TEMP); //CF26
  Ptr = Ptr + strlen(SYS_CONFIG_CF26_ENABLE_CAVITY_TEMP);
  *Ptr = (char)t_sys_data_current.enable_cavity_temp + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF27_TYPE_OF_THERMISTOR); //CF27
  Ptr = Ptr + strlen(SYS_CONFIG_CF27_TYPE_OF_THERMISTOR);
  *Ptr = (char)t_sys_data_current.enable_type_of_thermistor + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF28_ENABLE_HIGH_TEMP); //CF28
  Ptr = Ptr + strlen(SYS_CONFIG_CF28_ENABLE_HIGH_TEMP);
  *Ptr = (char)t_sys_data_current.enable_high_temp + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF29_UI_NUMBER); //CF29
  Ptr = Ptr + strlen(SYS_CONFIG_CF29_UI_NUMBER);
  *Ptr = (char)t_sys_data_current.ui_number + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF30_IS_2GT); //CF30
  Ptr = Ptr + strlen(SYS_CONFIG_CF30_IS_2GT);
  *Ptr = (char)t_sys_data_current.is_2GT + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF31_IS_MECHANISM_LEVEL); //CF31
  Ptr = Ptr + strlen(SYS_CONFIG_CF31_IS_MECHANISM_LEVEL);
  *Ptr = (char)t_sys_data_current.IsMechanismLevel + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF32_IS_LASER); //CF32
  Ptr = Ptr + strlen(SYS_CONFIG_CF32_IS_LASER);
  *Ptr = (char)t_sys_data_current.IsLaser + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF33_IS_LASER_MODE); //CF33
  Ptr = Ptr + strlen(SYS_CONFIG_CF33_IS_LASER_MODE);
  *Ptr = (char)t_sys_data_current.IsLaserMode + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF34); //CF34
  Ptr = Ptr + strlen(SYS_CONFIG_CF34);
  *Ptr = (char)t_sys_data_current.cf34 + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF35); //CF35
  Ptr = Ptr + strlen(SYS_CONFIG_CF35);
  *Ptr = (char)t_sys_data_current.cf35 + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF36); //CF36
  Ptr = Ptr + strlen(SYS_CONFIG_CF36);
  *Ptr = (char)t_sys_data_current.cf36 + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF37); //CF37
  Ptr = Ptr + strlen(SYS_CONFIG_CF37);
  *Ptr = (char)t_sys_data_current.cf37 + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF38); //CF38
  Ptr = Ptr + strlen(SYS_CONFIG_CF38);
  *Ptr = (char)t_sys_data_current.cf38 + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF39); //CF39
  Ptr = Ptr + strlen(SYS_CONFIG_CF39);
  *Ptr = (char)t_sys_data_current.cf39 + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF40); //CF40
  Ptr = Ptr + strlen(SYS_CONFIG_CF40);
  *Ptr = (char)t_sys_data_current.cf40 + '0';
}

/********************************************************************
* @brief   :
* @param   :无
* @retval  :
* @notice  ：
* @author  ：
* @date    ：
* @versions：
********************************************************************/
void SysConfigOperation::ChangeZMaxPosValueInfo(void)
{
  char *Ptr;
  char ZMaxPosValueBuf1[20];
  char ZMaxPosValueBuf2[20];//Z轴最大位置存储空间  共20字节

  //Z轴存储空间初始化为空白符
  for (int i = 0; i < 20; i++)
  {
    ZMaxPosValueBuf2[i] = ' ';
  }

  (void)snprintf(ZMaxPosValueBuf1, sizeof(ZMaxPosValueBuf1), "%s%f", SYS_CONFIG_CF07_Z_MAX_POS_VALUE, t_sys_data_current.poweroff_rec_z_max_value);
  (void)memcpy(ZMaxPosValueBuf2, ZMaxPosValueBuf1, strlen(ZMaxPosValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF07_Z_MAX_POS_VALUE); //CF8:0,0表示还没测量过行程
  (void)memcpy(Ptr, ZMaxPosValueBuf2, sizeof(ZMaxPosValueBuf2)); //覆盖原来的20个字节
}

/********************************************************************
* @brief   :
* @param   :无
* @retval  :
* @notice  ：
* @author  ：
* @date    ：
* @versions：
********************************************************************/
void SysConfigOperation::ChangeMatCheckAvgVolValueInfo(void)
{
  char *Ptr;
  char MatCheckAvgVolValueBuf1[20];
  char MatCheckAvgVolValueBuf2[20];//Z轴最大位置存储空间  共20字节

  //Z轴存储空间初始化为空白符
  for (int i = 0; i < 20; i++)
  {
    MatCheckAvgVolValueBuf2[i] = ' ';
  }

  (void)snprintf(MatCheckAvgVolValueBuf1, sizeof(MatCheckAvgVolValueBuf1), "%s%f", SYS_CONFIG_CF10_MAT_CHECK_VOL_VALUE, t_sys_data_current.material_chk_vol_value);
  (void)memcpy(MatCheckAvgVolValueBuf2, MatCheckAvgVolValueBuf1, strlen(MatCheckAvgVolValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF10_MAT_CHECK_VOL_VALUE); //CF10:0.5,0.5表示断料检测模块空载时的电压值
  (void)memcpy(Ptr, MatCheckAvgVolValueBuf2, sizeof(MatCheckAvgVolValueBuf2)); //覆盖原来的20个字节
}

void SysConfigOperation::ChangeBedLevelZAtLFValue(void)
{
  char *Ptr;
  char BedLevelZValueBuf1[20];
  char BedLevelZValueBuf2[20];//Z轴最大位置存储空间  共20字节

  //Z轴存储空间初始化为空白符
  for (int i = 0; i < 20; i++)
  {
    BedLevelZValueBuf2[i] = ' ';
  }

  (void)snprintf(BedLevelZValueBuf1, sizeof(BedLevelZValueBuf1), "%s%f", SYS_CONFIG_CF12_AUTO_BED_LEVEL_Z_LF, t_sys_data_current.bed_level_z_at_left_front);
  (void)memcpy(BedLevelZValueBuf2, BedLevelZValueBuf1, strlen(BedLevelZValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF12_AUTO_BED_LEVEL_Z_LF); //CF8:0,0表示还没测量过行程
  (void)memcpy(Ptr, BedLevelZValueBuf2, sizeof(BedLevelZValueBuf2)); //覆盖原来的20个字节
}

void SysConfigOperation::ChangeBedLevelZAtRFValue(void)
{
  char *Ptr;
  char BedLevelZValueBuf1[20];
  char BedLevelZValueBuf2[20];//Z轴最大位置存储空间  共20字节

  //Z轴存储空间初始化为空白符
  for (int i = 0; i < 20; i++)
  {
    BedLevelZValueBuf2[i] = ' ';
  }

  (void)snprintf(BedLevelZValueBuf1, sizeof(BedLevelZValueBuf1), "%s%f", SYS_CONFIG_CF13_AUTO_BED_LEVEL_Z_RF, t_sys_data_current.bed_level_z_at_right_front);
  (void)memcpy(BedLevelZValueBuf2, BedLevelZValueBuf1, strlen(BedLevelZValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF13_AUTO_BED_LEVEL_Z_RF); //CF8:0,0表示还没测量过行程
  (void)memcpy(Ptr, BedLevelZValueBuf2, sizeof(BedLevelZValueBuf2)); //覆盖原来的20个字节
}

void SysConfigOperation::ChangeBedLevelZAtLBValue(void)
{
  char *Ptr;
  char BedLevelZValueBuf1[20];
  char BedLevelZValueBuf2[20];//Z轴最大位置存储空间  共20字节

  //Z轴存储空间初始化为空白符
  for (int i = 0; i < 20; i++)
  {
    BedLevelZValueBuf2[i] = ' ';
  }

  (void)snprintf(BedLevelZValueBuf1, sizeof(BedLevelZValueBuf1), "%s%f", SYS_CONFIG_CF14_AUTO_BED_LEVEL_Z_LB, t_sys_data_current.bed_level_z_at_left_back);
  (void)memcpy(BedLevelZValueBuf2, BedLevelZValueBuf1, strlen(BedLevelZValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF14_AUTO_BED_LEVEL_Z_LB); //CF8:0,0表示还没测量过行程
  (void)memcpy(Ptr, BedLevelZValueBuf2, sizeof(BedLevelZValueBuf2)); //覆盖原来的20个字节
}

void SysConfigOperation::ChangeBedLevelZAtRBValue(void)
{
  char *Ptr;
  char BedLevelZValueBuf1[20];
  char BedLevelZValueBuf2[20];//Z轴最大位置存储空间  共20字节

  //Z轴存储空间初始化为空白符
  for (int i = 0; i < 20; i++)
  {
    BedLevelZValueBuf2[i] = ' ';
  }

  (void)snprintf(BedLevelZValueBuf1, sizeof(BedLevelZValueBuf1), "%s%f", SYS_CONFIG_CF15_AUTO_BED_LEVEL_Z_RB, t_sys_data_current.bed_level_z_at_right_back);
  (void)memcpy(BedLevelZValueBuf2, BedLevelZValueBuf1, strlen(BedLevelZValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF15_AUTO_BED_LEVEL_Z_RB); //CF8:0,0表示还没测量过行程
  (void)memcpy(Ptr, BedLevelZValueBuf2, sizeof(BedLevelZValueBuf2)); //覆盖原来的20个字节
}

void SysConfigOperation::ChangeBedLevelZAtMiddleValue(void)
{
  char *Ptr;
  char BedLevelZValueBuf1[20];
  char BedLevelZValueBuf2[20];//Z轴最大位置存储空间  共20字节

  //Z轴存储空间初始化为空白符
  for (int i = 0; i < 20; i++)
  {
    BedLevelZValueBuf2[i] = ' ';
  }

  (void)snprintf(BedLevelZValueBuf1, sizeof(BedLevelZValueBuf1), "%s%f", SYS_CONFIG_CF16_AUTO_BED_LEVEL_Z_MIDDLE, t_sys_data_current.bed_level_z_at_middle);
  (void)memcpy(BedLevelZValueBuf2, BedLevelZValueBuf1, strlen(BedLevelZValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF16_AUTO_BED_LEVEL_Z_MIDDLE); //CF8:0,0表示还没测量过行程
  (void)memcpy(Ptr, BedLevelZValueBuf2, sizeof(BedLevelZValueBuf2)); //覆盖原来的20个字节
}

void SysConfigOperation::ChangePidOutputFactorValue(void)
{
  char *Ptr;
  char PidOutputFactorValueBuf1[20];
  char PidOutputFactorValueBuf2[20];//Z轴最大位置存储空间  共20字节

  //Z轴存储空间初始化为空白符
  for (int i = 0; i < 20; i++)
  {
    PidOutputFactorValueBuf2[i] = ' ';
  }

  (void)snprintf(PidOutputFactorValueBuf1, sizeof(PidOutputFactorValueBuf1), "%s%f", SYS_CONFIG_CF17_PID_OUTPUT_FACTOR, t_sys_data_current.pid_output_factor);
  (void)memcpy(PidOutputFactorValueBuf2, PidOutputFactorValueBuf1, strlen(PidOutputFactorValueBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF17_PID_OUTPUT_FACTOR); //CF8:0,0表示还没测量过行程
  (void)memcpy(Ptr, PidOutputFactorValueBuf2, sizeof(PidOutputFactorValueBuf2)); //覆盖原来的20个字节
}

void SysConfigOperation::ChangeIsSoftfilament(void)
{
  char *Ptr;
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF19_IS_SOFTFILAMENT);//CF19:1打开软料；0关闭软料
  Ptr = Ptr + strlen(SYS_CONFIG_CF19_IS_SOFTFILAMENT);
  *Ptr = (char)t_sys_data_current.enable_soft_filament + '0';
}

//void SysConfigOperation::ChangeLOGOinterface(void)
//{
//  char *Ptr;
//  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF20_LOGOorNO); //CF20:1打开开机logo显示；0关闭开机logo显示
//  Ptr = Ptr + strlen(SYS_CONFIG_CF20_LOGOorNO);
//  *Ptr = (char)t_sys_data_current.enable_LOGO_interface + '0';
//}

void SysConfigOperation::ChangeLogoID(void)
{
  //  char *Ptr;
  //  char logoIDBuf1[10];
  //  char logoIDBuf2[10];//Z轴最大位置存储空间  共20字节
  //  //Z轴存储空间初始化为空白符
  //  for (int i = 0; i < 10; i++)
  //  {
  //    logoIDBuf2[i] = ' ';
  //  }
  //  (void)snprintf(logoIDBuf1, sizeof(logoIDBuf1), "%s%d", SYS_CONFIG_CF21_LOGOID, t_sys_data_current.logo_id);
  //  (void)memcpy(logoIDBuf2, logoIDBuf1, strlen(logoIDBuf1));
  //  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF21_LOGOID); //CF21:
  //  (void)memcpy(Ptr, logoIDBuf2, sizeof(logoIDBuf2)); //覆盖原来的10个字节
  /*目前id数小于10，所以用这种方法可以正常配置logo图片*/
  char *Ptr;
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF21_LOGOID); //CF21:
  Ptr = Ptr + strlen(SYS_CONFIG_CF21_LOGOID);
  *Ptr = (char)t_sys_data_current.logo_id + '0';
}

void SysConfigOperation::ChangeCustomModelId(void)
{
  //  char *Ptr;
  //  char customModelIdBuf1[10];
  //  char customModelIdBuf2[10];//Z轴最大位置存储空间  共20字节
  //  //Z轴存储空间初始化为空白符
  //  for (int i = 0; i < 10; i++)
  //  {
  //    customModelIdBuf2[i] = ' ';
  //  }
  //  (void)snprintf(customModelIdBuf1, sizeof(customModelIdBuf1), "%s%d", SYS_CONFIG_CF22_CUSTOM_MODEL_ID, t_sys_data_current.custom_model_id);
  //  (void)memcpy(customModelIdBuf2, customModelIdBuf1, strlen(customModelIdBuf1));
  //  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF22_CUSTOM_MODEL_ID); //CF22:
  //  (void)memcpy(Ptr, customModelIdBuf2, sizeof(customModelIdBuf2)); //覆盖原来的10个字节
  char *Ptr;
  char customModelIdBuf1[10];
  char customModelIdBuf2[10];//机型字符串存储空间  共10字节

  //机型字符串存储空间初始化为空白符
  for (int i = 0; i < 10; i++)
  {
    customModelIdBuf2[i] = ' ';
  }

  (void)snprintf(customModelIdBuf1, sizeof(customModelIdBuf1), "%s%d", SYS_CONFIG_CF22_CUSTOM_MODEL_ID, t_sys_data_current.custom_model_id);
  (void)memcpy(customModelIdBuf2, customModelIdBuf1, strlen(customModelIdBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF22_CUSTOM_MODEL_ID); //CF4:0,0表示M14
  (void)memcpy(Ptr, customModelIdBuf2, sizeof(customModelIdBuf2)); //覆盖原来的10个字节
}

/********************************************************************
* @brief   :ChangelogoInfo
* @param   :改变logo图片
* @retval  :
* @notice  ：
* @author  ：john
* @date    ：2017/5/10
* @versions：
********************************************************************/
void SysConfigOperation::ChangelogoInfo(void)
{
  char *Ptr;
  //  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF20_LOGOorNO); //CF20:1,打开开机logo显示 0为关闭logo显示
  //  Ptr = Ptr + strlen(SYS_CONFIG_CF20_LOGOorNO);
  //  *Ptr = (char)t_sys_data_current.enable_LOGO_interface + '0';
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF21_LOGOID); //CF21:logo图片的id号
  Ptr = Ptr + strlen(SYS_CONFIG_CF21_LOGOID);
  *Ptr = (char)t_sys_data_current.logo_id + '0';
}

/********************************************************************
* @brief   :ChangeBezzerSound
* @param   :改变保存的蜂鸣器提示音
* @retval  :
* @notice  ：
* @author  ：john
* @date    ：2017/7/31
* @versions：V1.0
********************************************************************/
void SysConfigOperation::ChangeBezzerSound(void)
{
  char *Ptr;
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF23_CUSTOM_BEZZERSOUND); //CF23:11表示按键和警报音关闭；22表示按键和警报音开启
  Ptr = Ptr + strlen(SYS_CONFIG_CF23_CUSTOM_BEZZERSOUND);
  *Ptr = (char)((t_sys.key_sound + 1) + '0');//保存的值不能是字符‘0’
  Ptr += 1;
  *Ptr = (char)((t_sys.alarm_sound + 1) + '0');//防止提取保存值时把0转化没了
}

void SysConfigOperation::ChangeZOffsetZeroValue(void)
{
  char *Ptr;
  char ZOffsetZeroBuf1[20];
  char ZOffsetZeroBuf2[20];//Z轴最大位置存储空间  共20字节

  //Z轴存储空间初始化为空白符
  for (int i = 0; i < 20; i++)
  {
    ZOffsetZeroBuf2[i] = ' ';
  }

  (void)snprintf(ZOffsetZeroBuf1, sizeof(ZOffsetZeroBuf1), "%s%f", SYS_CONFIG_CF24_Z_OFFSET_ZERO_VALUE, t_sys_data_current.z_offset_value);
  (void)memcpy(ZOffsetZeroBuf2, ZOffsetZeroBuf1, strlen(ZOffsetZeroBuf1));
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF24_Z_OFFSET_ZERO_VALUE); //CF8:0,0表示还没测量过行程
  (void)memcpy(Ptr, ZOffsetZeroBuf2, sizeof(ZOffsetZeroBuf2)); //覆盖原来的20个字节
}

#if LASER_MODE
void SysConfigOperation::ChangeLaser(void)
{
  char *Ptr;
  Ptr = strstr((char *)sys_data, SYS_CONFIG_CF32_IS_LASER); //CF23:11表示按键和警报音关闭；22表示按键和警报音开启
  Ptr = Ptr + strlen(SYS_CONFIG_CF32_IS_LASER);
  *Ptr = (char)(t_sys_data_current.IsLaser + '0');//保存的值不能是字符‘0’
}
#endif

/********************************************************************
* @brief   : 打印机的一些系统信息配置
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/23 PM
* @versions：V1.0
********************************************************************/
SysConfig::SysConfig()
{
  /*************机器信息配置***********/
  t_sys_data_current.model_id = DEFAULT_MODEL;                                           // 默认配置，机型的选择 （在machinecustom.h文件中修改）
  t_sys_data_current.pic_id = PICTURE_IS_CHINESE;                                   // 图片语言选择 ：中文 、日文、英文、韩文、俄文（宏定义在machinecustom.h文件）
  t_sys_data_current.enable_color_mixing = DEFAULT_COLORMIXING;                         // 是否打开混色功能（在machinecustom.h文件中修改）
  t_sys_data_current.enable_powerOff_recovery = DEFAULT_POWEROFFRECOVERY;               // 是否打开断电续打功能（在machinecustom.h文件中修改）
  t_sys_data_current.enable_material_check = DEFAULT_MATCHECK;                          // 是否打开断料检测功能（在machinecustom.h文件中修改）
  t_sys_data_current.have_set_machine = false;                                          // 是否已经配置机型
  t_sys_data_current.poweroff_rec_z_max_value = 0.0f;                                  // 断电z最大高度初始化
  t_sys_data_current.material_chk_vol_value = 0.0f;                                      // 断料检测模块空载时的平均电压初始化
  t_sys_data_current.enable_block_detect = 0;                                           // 是否是能块检测（ 1：使能 ，0：不使能）
  t_sys_data_current.enable_soft_filament = 0;                                                // 是否使用软料（1：使用，0：不使用）
  t_sys_data_current.enable_LOGO_interface = 0;                                                      // 开机是否显示LOGO（1：显示，0：不显示）
  t_sys_data_current.logo_id = 0;                                                        // LOGO图片ID
  t_sys_data_current.custom_model_id = 0;
  memset(build_date_str, 0, sizeof(build_date_str));                 // 初始化FWBuildDateBuf内存全为0 ( )
  memset(boot_ver_str, 0, sizeof(boot_ver_str));             // 初始化FWBootVersionBuf内存全为0 ( )
  memset(app2_ver_str, 0, sizeof(app2_ver_str));             // 初始化FWApp2VersionBuf内存全为0 ( )
  t_sys_data_current.bed_level_z_at_left_front = 0.0f;
  t_sys_data_current.bed_level_z_at_right_front = 0.0f;
  t_sys_data_current.bed_level_z_at_left_back = 0.0f;
  t_sys_data_current.bed_level_z_at_right_back = 0.0f;
  t_sys_data_current.bed_level_z_at_middle = 0.0f;
  t_sys_data_current.pid_output_factor = 1.0f;
  t_sys_data_current.enable_bed_level = 0;
  t_sys_data_current.z_offset_value = 0.0f;
  t_sys_data_current.enable_v5_extruder = 0;
  t_sys_data_current.enable_cavity_temp = 0;
  // 热敏电阻类型：
  // 0、普通热敏电阻 最高温度300度
  // 1、热电偶 660度对应3.3V
  t_sys_data_current.enable_type_of_thermistor = 0;
  t_sys_data_current.enable_high_temp = 0;
  t_sys_data_current.ui_number = 0;
  t_sys_data_current.is_2GT = 0;
  t_sys_data_current.IsMechanismLevel = 0;
  t_sys_data_current.IsLaser = 0;
  t_sys_data_current.IsLaserMode = 0;
  t_sys_data_current.cf34 = 0;
  t_sys_data_current.cf35 = 0;
  t_sys_data_current.cf36 = 0;
  t_sys_data_current.cf37 = 0;
  t_sys_data_current.cf38 = 0;
  t_sys_data_current.cf39 = 0;
  t_sys_data_current.cf40 = 0;
  /***********机器信息内存初始化*********/
  memset(t_sys.model_str, 0, sizeof(t_sys.model_str));                             // 初始化modelStr内存全为0 （机型信息）
  memset(t_sys.function_str, 0, sizeof(t_sys.function_str));                       // 初始化functionStr内存全为0 （功能信息）
  memset(t_sys.version_str, 0, sizeof(t_sys.version_str));                         // 初始化versionStr内存全为0 （版本信息）
  t_sys.is_bed_level_down_to_zero = 0;
  t_sys.is_detect_extruder_thermistor = 0;
  t_sys.enable_automatic_refueling = 0;
  t_sys.is_planner_slow_down = 0;
  t_sys.serial_moves_queued = 0;
  t_sys.lcd_ssd1963_43_480_272 = 0;
  t_sys.print_time_save = 0;
  t_sys.is_granulator = 0;
  t_sys.pulse_delay_time = 0;
  t_sys.enable_color_buf = 0;
}

/********************************************************************
* @brief   : 系统信息配置初始化
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/23 PM
* @versions：V1.0
********************************************************************/
void SysConfig::init(void)
{
  sys_read_info_from_sd(sys_config_file_path);                                                  //从SD卡读取配置信息
  explainInfo();                                                     //解析SD卡中的配置信息
  sysconfigLog();                                                    //调试打印配置信息

  if (!t_sys_data_current.have_set_machine) // 没有设置机器时，混色模式打开，测试固件可以测试B轴电机
  {
    t_sys_data_current.enable_color_mixing = true;
  }

  t_sys.key_sound = (t_sys_data_current.buzzer_value / 10) - 1;                                  // 配置按键提示音
  t_sys.alarm_sound = (t_sys_data_current.buzzer_value % 10) - 1;                                // 配置按键提示音

  if (!t_sys_data_current.enable_LOGO_interface)
    t_sys_data_current.logo_id = 0;
}

/********************************************************************
* @brief   : 解析从SD卡读取到的数据
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/23 PM
* @versions：V1.0
********************************************************************/
void SysConfig::explainInfo(void)
{
  getFWBootVersion();                                                // CF0 解析保存固件Boorloader版本
  getFWApp2Version();                                                // CF1 解析保存固件当前版本信息
  getGetFWBuildDate();                                               // CF2 解析保存固件建立日期
  getMachineSettingMark();                                           // CF3 解析是否已配置机器 1:YES-已设置过机器，不显示配置界面 0:NO-还没设置过机器，显示配置界面
  getModelId();                                                      // CF4 解析机型 0:M14 1:M2030 2:M2041 3:M2048 4:M3145 5:M4141 6:M4040 7:M4141S 8:AMP410W 9:M14R03 10:M2030HY 11:M14S 12:M3145S
  getModelPicId();                                                   // CF5 解析界面语种  1:中文图片  2：日文图片
  getPowerOffRecoveryMask();                                         // CF6 解析断电续打功能是否开启
  getPowerOffRecoveryZMaxValue();                                    // CF7 解析Z轴最大位置
  getColorMixingMask();                                              // CF8 解析是否打开混色功能
  getMaterialCheckMask();                                            // CF9 解析是否打开断料检测功能
  getMaterialCheckVolValue();                                        // CF10 解析断料检测模块电压校准值,空载默认值设为0.5V
  getBlockDetectMask();                                              // CF11 解析是否打开块检测功能
  getBedLevelZAtLF();                                                // CF12
  getBedLevelZAtRF();                                                // CF13
  getBedLevelZAtLB();                                                // CF14
  getBedLevelZAtRB();                                                // CF15
  getBedLevelZAtMiddle();                                            // CF16
  getPidOutputFactor();                                              // CF17
  getAutoBedLevelMask();                                             // CF18
  getisSoftfilament();                                               // CF19 解析是否使用了软料
  getLOGOinterface();                                                // CF20 解析是否开启开机LOGO
  getlogoID();                                                       // CF21 解析SD卡中保存的LOGOid
  getCustomModelId();                                                // CF22
  getBezzerSound();                                                  // CF23 获取蜂鸣器保存值
  getZOffsetZeroValue();                                             // CF24
  getV5ExtruderMask();                                               // CF25
  getCF26();
  getCF27();
  getCF28();
  getCF29();
  getCF30();
  getCF31();
  getCF32();
  getCF33();
  getCF34();
  getCF35();
  getCF36();
  getCF37();
  getCF38();
  getCF39();
  getCF40();
  getModelStr();                                                     // 通过从SD卡读取的机型信息编号查找将机型信息编号写进机型数组中
  getStatusInfoStr();                                                // 将从SD卡读取的功能和版本信息分别写进功能和版本数组中

  if (2 == t_sys_data_current.enable_bed_level)
  {
    if (1U == t_sys_data_current.IsMechanismLevel)
      t_sys.is_bed_level_down_to_zero = 1;
    else
      t_sys.is_bed_level_down_to_zero = 0;
  }

  if (1 == t_sys_data_current.ui_number) // 4.3寸蓝屏界面
  {
    TextDisplayX = 235;
  }
  else if (2 == t_sys_data_current.ui_number) // 4.3寸黑屏界面
  {
    TextDisplayX = 210;
  }
  else if(0 == t_sys_data_current.ui_number) // 非4.3寸
  {
    TextDisplayX = 235;
  }

  if (!t_sys_data_current.IsLaserMode)
    t_sys_data_current.IsLaser = 0;

  //  if(is_sys_data_add)
  //  {
  //    sys_save_data(sys_config_file_path);
  //    is_sys_data_add = false;
  //  }
}

/********************************************************************
* @brief   : 查找key值的后一个位置
* @param   : 无
* @retval  : 无
* @notice  ：读取的是KEY后面的数据 （ 如 ：CF0: 1 ,在这里面需要读取的是 1 的位置 ）
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
char *SysConfig::find_key_value(const char *key)
{
  static char *value;                                                // 用于保存需要返回查找信息的位置
  value = strstr((char *)sys_data, key);                              // 返回key后面数据的指针位置

  if (value != NULL)                                                 // 判断是否查找到匹配信息
    value = value + strlen(key);                                     // 计算需要查找信息后面数据的首地址

  return value;                                                      // 返回数据首地址
}

/********************************************************************
* @brief   : 解析保存Boorloader版本
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getFWBootVersion(void)
{
  static char *VersionInfoHeadPtr;                                   //版本信息头部
  static char *VersionInfoEndPtr;                                    //版本信息尾部
  int length = 0;
  VersionInfoHeadPtr = find_key_value(SYS_CONFIG_CF00_BOOT_VERSION);      //读取 SYS_CONFIG_CF00_BOOT_VERSION( "CF0:" ) 在 InfoBuf 中的索引号

  /**********判断是否有匹配数据************/
  if (VersionInfoHeadPtr != NULL)                                    //判断是否有匹配数据
  {
    /***********有匹配数据情况************/
    VersionInfoEndPtr = strchr(VersionInfoHeadPtr, '\r');            //查找SYS_CONFIG_BOOT_VERSION( "CF0:" )后面数据的结束位置
    length = VersionInfoEndPtr - VersionInfoHeadPtr;                 //计算SYS_CONFIG_BOOT_VERSION( "CF0:" )后面数据的字节数
    (void)strncpy(boot_ver_str, VersionInfoHeadPtr, (unsigned int)length); //将 Boorloader版本 信息的编号复制出来
    boot_ver_str[length] = 0;
    char *Ptr = strstr((char *)boot_ver_str, " ");               //补0 ，表示字符串结束

    if (Ptr != NULL)
    {
      *Ptr = 0;                                                      //删除后面多余的空白符
    }
  }
  else                                                               //没有数据匹配的情况
  {
    /*********没有数据匹配的情况*********/
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF00_BOOT_VERSION");    //报错 。没有发现bootload版本信息的配置
    //    sys_add_data(SYS_CONFIG_CF00_BOOT_VERSION, "V3.7");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析保存当前版本信息
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getFWApp2Version(void)
{
  static char *VersionInfoHeadPtr;                                   //版本信息头部
  static char *VersionInfoEndPtr;                                    //版本信息尾部
  int length = 0;
  VersionInfoHeadPtr = find_key_value(SYS_CONFIG_CF01_APP2_VERSION);      //读取 SYS_CONFIG_CF01_APP2_VERSION( "CF1:" ) 在 InfoBuf 中的索引号

  /**********判断是否有匹配数据************/
  if (VersionInfoHeadPtr != NULL)                                    //判断是否有匹配数据
  {
    /***********有匹配数据情况************/
    VersionInfoEndPtr = strchr(VersionInfoHeadPtr, '\r');            //查找SYS_CONFIG_APP2_VERSION( "CF1:" )后面数据的结束位置
    length = VersionInfoEndPtr - VersionInfoHeadPtr;                 //计算SYS_CONFIG_APP2_VERSION( "CF1:" )后面数据的字节数
    (void)strncpy(app2_ver_str, VersionInfoHeadPtr, (unsigned int)length);  //将当前版本信息编号复制出来
    app2_ver_str[length] = 0;
  }
  else
  {
    /*********没有数据匹配的情况*********/
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF01_APP2_VERSION");    //报错 。没有发现现在版本信息的配置
    //    sys_add_data(SYS_CONFIG_CF01_APP2_VERSION, "V3.1.6");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析保存固件建立日期
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getGetFWBuildDate(void)
{
  static char *DateInfoPtr;
  DateInfoPtr = find_key_value(SYS_CONFIG_CF02_DATE);                     //读取 SYS_CONFIG_CF02_DATE( "CF2:" ) 在 InfoBuf 中的索引号

  /**********判断是否有匹配数据************/
  if (DateInfoPtr != NULL)                                           //判断是否有匹配数据
  {
    /***********有匹配数据情况************/
    (void)strncpy(build_date_str, DateInfoPtr, 8);                   //将固件建立日期时间复制出来                                         //判断是否有匹配数据
    build_date_str[8] = 0; //20151110                                // 字符串结束字符
  }
  else
  {
    /*********没有数据匹配的情况*********/
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF02_DATE");               //报错 。没有发现固件建立日期时间的配置
    //    sys_add_data(SYS_CONFIG_CF02_DATE, "20180121");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析是否已配置机器 1:YES-已设置过机器，不显示配置界面 0:NO-还没设置过机器，显示配置界面
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getMachineSettingMark(void)
{
  static char *MachineSettingPtr;
  MachineSettingPtr = find_key_value(SYS_CONFIG_CF03_MACHINE_SETTING_MASK);    //读取 SYS_CONFIG_CF03_MACHINE_SETTING_MASK( "CF3:" ) 在 InfoBuf 中的索引号

  if (MachineSettingPtr != NULL)                                          //判断是否有匹配数据
    t_sys_data_current.have_set_machine = (unsigned char)(*MachineSettingPtr - '0');         //计算查看是否已经配置过机器，用于决定是否显示配置界面
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF03_MACHINE_SETTING_MASK");//报错 。没有发现是否已配置机器的配置
    //    sys_add_data(SYS_CONFIG_CF03_MACHINE_SETTING_MASK, "1");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析机型 0:M14 1:M2030 2:M2041 3:M2048 4:M3145 5:M4141 6:M4040 7:M4141S 8:AMP410W 9:M14R03 10:M2030HY 11:M14S 12:M3145S
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getModelId(void)
{
  static char *ModelInfoPtr;
  ModelInfoPtr = find_key_value(SYS_CONFIG_CF04_MODEL_ID);               //SYS_CONFIG_CF04_MODEL_ID( "CF4:" ) 在 InfoBuf 中的索引号

  if (ModelInfoPtr != NULL)                                          //判断是否有匹配数据
    t_sys_data_current.model_id = atoi(ModelInfoPtr);                                    //字符串转数字
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF04_MODEL_ID");        //报错 。没有发现机型的配置
    //    sys_add_data(SYS_CONFIG_CF04_MODEL_ID, "0");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析界面语种  1:中文图片  2：日文图片
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getModelPicId(void)
{
  static char *PictureInfoPtr;
  PictureInfoPtr = find_key_value(SYS_CONFIG_CF05_PICTURE_ID);            //读取 SYS_CONFIG_CF05_PICTURE_ID( "CF5:" ) 在 InfoBuf 中的索引号

  if (PictureInfoPtr != NULL)
    t_sys_data_current.pic_id = (unsigned char)(*PictureInfoPtr - '0');             //计算保存界面语种信息
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF05_PICTURE_ID");      //报错 。没有发现界面语种信息的配置
    //    sys_add_data(SYS_CONFIG_CF05_PICTURE_ID, "1");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析断电续打功能是否开启
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getPowerOffRecoveryMask(void)
{
  static char *PowerOffRecoveryInfoPtr;
  PowerOffRecoveryInfoPtr = find_key_value(SYS_CONFIG_CF06_POWEROFF_RECOVERY_MASK);  //读取 SYS_CONFIG_CF06_POWEROFF_RECOVERY_MASK( "CF6:" ) 在 InfoBuf 中的索引号

  if (PowerOffRecoveryInfoPtr != NULL)
    t_sys_data_current.enable_powerOff_recovery = (unsigned char)(*PowerOffRecoveryInfoPtr - '0'); //计算保存是否打开断电续打功能
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF06_POWEROFF_RECOVERY_MASK");     //报错 。没有发现是否打开断电续打功能的配置
    //    sys_add_data(SYS_CONFIG_CF06_POWEROFF_RECOVERY_MASK, "0");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析Z轴最大位置
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getPowerOffRecoveryZMaxValue(void)
{
  static char *ZMaxPosValueInfoPtr;
  ZMaxPosValueInfoPtr = find_key_value(SYS_CONFIG_CF07_Z_MAX_POS_VALUE);  //读取 SYS_CONFIG_CF07_Z_MAX_POS_VALUE( "CF7:" ) 在 InfoBuf 中的索引号

  if (ZMaxPosValueInfoPtr != NULL)
    t_sys_data_current.poweroff_rec_z_max_value = (float)atof(ZMaxPosValueInfoPtr);    //字符串转浮点数 ，计算保存Z轴最大位置
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF07_Z_MAX_POS_VALUE"); //报错 。没有发现Z轴最大位置的配置
    //    sys_add_data(SYS_CONFIG_CF07_Z_MAX_POS_VALUE, "0.0");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析是否打开混色功能
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getColorMixingMask(void)
{
  static char *ColorMixingInfoPtr;
  ColorMixingInfoPtr = find_key_value(SYS_CONFIG_CF08_COLOR_MIXING_MASK);   //读取 SYS_CONFIG_CF08_COLOR_MIXING_MASK( "CF8:" ) 在 InfoBuf 中的索引号

  if (ColorMixingInfoPtr != NULL)
  {
    t_sys_data_current.enable_color_mixing = (unsigned char)(*ColorMixingInfoPtr - '0');  //计算保存是否打开混色功能
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF08_COLOR_MIXING_MASK"); //报错 。没有发现是否打开混色功能的配置
    //    sys_add_data(SYS_CONFIG_CF08_COLOR_MIXING_MASK, "0");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析是否打开断料检测功能
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getMaterialCheckMask(void)
{
  static char *MatCheckInfoPtr;
  MatCheckInfoPtr = find_key_value(SYS_CONFIG_CF09_MAT_CHECK_MASK);       //读取 SYS_CONFIG_CF09_MAT_CHECK_MASK( "CF9:" ) 在 InfoBuf 中的索引号

  if (MatCheckInfoPtr != NULL)
  {
    t_sys_data_current.enable_material_check = (unsigned char)(*MatCheckInfoPtr - '0'); //计算保存是否打开断料检测功能
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF09_MAT_CHECK_MASK");  //报错 。没有发现是否打开断料检测的配置
    //    sys_add_data(SYS_CONFIG_CF09_MAT_CHECK_MASK, "0");
    //    is_sys_data_add = true;
  }
}

void SysConfig::getAutoBedLevelMask(void)
{
  static char *BedLevelInfoPtr;
  BedLevelInfoPtr = find_key_value(SYS_CONFIG_CF18_AUTO_BED_LEVEL_MASK);       //读取 SYS_CONFIG_CF18_AUTO_BED_LEVEL_MASK( "CF9:" ) 在 InfoBuf 中的索引号

  if (BedLevelInfoPtr != NULL)
  {
    t_sys_data_current.enable_bed_level = (unsigned char)(*BedLevelInfoPtr - '0'); //计算保存是否打开断料检测功能
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF18_AUTO_BED_LEVEL_MASK");  //报错 。没有发现是否打开断料检测的配置
    //    sys_add_data(SYS_CONFIG_CF18_AUTO_BED_LEVEL_MASK, "0");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析断料检测模块电压校准值,空载默认值设为0.5V
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getMaterialCheckVolValue(void)
{
  static char *MatCheckVolValueInfoPtr;
  MatCheckVolValueInfoPtr = find_key_value(SYS_CONFIG_CF10_MAT_CHECK_VOL_VALUE);    //读取 SYS_CONFIG_CF10_MAT_CHECK_VOL_VALUE( "CF10:" ) 在 InfoBuf 中的索引号

  if (MatCheckVolValueInfoPtr != NULL)
  {
    t_sys_data_current.material_chk_vol_value = (float)atof(MatCheckVolValueInfoPtr);              //字符串转成浮点数据 ，计算保存断料检测模块电压校准值
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF10_MAT_CHECK_VOL_VALUE");       //报错 。没有发现断料检测模块电压校准值的配置
    //    sys_add_data(SYS_CONFIG_CF10_MAT_CHECK_VOL_VALUE, "0.5");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 解析是否打开块检测功能
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getBlockDetectMask(void)
{
  static char *BlockDetectInfoPtr;
  BlockDetectInfoPtr = find_key_value(SYS_CONFIG_CF11_BLOCK_DETECT_MASK);   //读取 SYS_CONFIG_CF11_BLOCK_DETECT_MASK( "CF11:" ) 在 InfoBuf 中的索引号

  if (BlockDetectInfoPtr != NULL)
  {
    t_sys_data_current.enable_block_detect = (unsigned char)(*BlockDetectInfoPtr - '0');  //计算保存是否打开块检测功能
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF11_BLOCK_DETECT_MASK"); //报错 。没有发现是否打开块检测功能的配置
    //    sys_add_data(SYS_CONFIG_CF11_BLOCK_DETECT_MASK, "0");
    //    is_sys_data_add = true;
  }
}

void SysConfig::getBedLevelZAtLF(void)
{
  static char *BedLevelZInfoPtr;
  BedLevelZInfoPtr = find_key_value(SYS_CONFIG_CF12_AUTO_BED_LEVEL_Z_LF);

  if (BedLevelZInfoPtr != NULL)
  {
    t_sys_data_current.bed_level_z_at_left_front = (float)atof(BedLevelZInfoPtr);
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF12_AUTO_BED_LEVEL_Z_LF");
    //    sys_add_data(SYS_CONFIG_CF12_AUTO_BED_LEVEL_Z_LF, "0.0");
    //    is_sys_data_add = true;
  }
}

void SysConfig::getBedLevelZAtRF(void)
{
  static char *BedLevelZInfoPtr;
  BedLevelZInfoPtr = find_key_value(SYS_CONFIG_CF13_AUTO_BED_LEVEL_Z_RF);

  if (BedLevelZInfoPtr != NULL)
  {
    t_sys_data_current.bed_level_z_at_right_front = (float)atof(BedLevelZInfoPtr);
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF13_AUTO_BED_LEVEL_Z_RF");
    //    sys_add_data(SYS_CONFIG_CF13_AUTO_BED_LEVEL_Z_RF, "0.0");
    //    is_sys_data_add = true;
  }
}

void SysConfig::getBedLevelZAtLB(void)
{
  static char *BedLevelZInfoPtr;
  BedLevelZInfoPtr = find_key_value(SYS_CONFIG_CF14_AUTO_BED_LEVEL_Z_LB);

  if (BedLevelZInfoPtr != NULL)
  {
    t_sys_data_current.bed_level_z_at_left_back  = (float)atof(BedLevelZInfoPtr);
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF14_AUTO_BED_LEVEL_Z_LB");
    //    sys_add_data(SYS_CONFIG_CF14_AUTO_BED_LEVEL_Z_LB, "0.0");
    //    is_sys_data_add = true;
  }
}

void SysConfig::getBedLevelZAtRB(void)
{
  static char *BedLevelZInfoPtr;
  BedLevelZInfoPtr = find_key_value(SYS_CONFIG_CF15_AUTO_BED_LEVEL_Z_RB);

  if (BedLevelZInfoPtr != NULL)
  {
    t_sys_data_current.bed_level_z_at_right_back = (float)atof(BedLevelZInfoPtr);
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF15_AUTO_BED_LEVEL_Z_RB");
    //    sys_add_data(SYS_CONFIG_CF15_AUTO_BED_LEVEL_Z_RB, "0.0");
    //    is_sys_data_add = true;
  }
}

void SysConfig::getBedLevelZAtMiddle(void)
{
  static char *BedLevelZInfoPtr;
  BedLevelZInfoPtr = find_key_value(SYS_CONFIG_CF16_AUTO_BED_LEVEL_Z_MIDDLE);

  if (BedLevelZInfoPtr != NULL)
  {
    t_sys_data_current.bed_level_z_at_middle = (float)atof(BedLevelZInfoPtr);
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF16_AUTO_BED_LEVEL_Z_MIDDLE");
    //    sys_add_data(SYS_CONFIG_CF16_AUTO_BED_LEVEL_Z_MIDDLE, "0.0");
    //    is_sys_data_add = true;
  }
}

void SysConfig::getPidOutputFactor(void)
{
  static char *PidOutputFactorInfoPtr;
  PidOutputFactorInfoPtr = find_key_value(SYS_CONFIG_CF17_PID_OUTPUT_FACTOR);

  if (PidOutputFactorInfoPtr != NULL)
  {
    t_sys_data_current.pid_output_factor = (float)atof(PidOutputFactorInfoPtr);
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF17_PID_OUTPUT_FACTOR");
    //    sys_add_data(SYS_CONFIG_CF17_PID_OUTPUT_FACTOR, "1.0");
    //    is_sys_data_add = true;
  }

  if ((t_sys_data_current.pid_output_factor > ((1.0f / 0.85f) + 0.1f)) || (t_sys_data_current.pid_output_factor <= 0.0f)) //1/0.85+0.1时才不会更改最大系数1/0.85
  {
    t_sys_data_current.pid_output_factor = 1.0f;
  }
}
/********************************************************************
* @brief   : 解析是否使用软料
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：John
* @date    ：2017/04/27 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getisSoftfilament(void)
{
  static char *SoftfilamentInfoPtr;
  SoftfilamentInfoPtr = find_key_value(SYS_CONFIG_CF19_IS_SOFTFILAMENT);   //读取 SYS_CONFIG_CF19_IS_SOFTFILAMENT( "CF19:" ) 在 InfoBuf 中的索引号

  if (SoftfilamentInfoPtr != NULL)
  {
    t_sys_data_current.enable_soft_filament = (unsigned char)(*SoftfilamentInfoPtr - '0');  //计算保存是否使用了软料
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF19_IS_SOFTFILAMENT"); //报错 。没有发现是否打开软料功能的配置
    //    sys_add_data(SYS_CONFIG_CF19_IS_SOFTFILAMENT, "0");
    //    is_sys_data_add = true;
  }
}
/********************************************************************
* @brief   : 解析是否开启开机LOGO界面
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：John
* @date    ：2017/05/05 AM
* @versions：V1.0
********************************************************************/
void SysConfig::getLOGOinterface(void)
{
  static char *LOGOinterfaceInfoPtr;
  LOGOinterfaceInfoPtr = find_key_value(SYS_CONFIG_CF20_LOGOorNO);   //读取 SYS_CONFIG_LOGOorNO( "CF20:" ) 在 InfoBuf 中的索引号

  if (LOGOinterfaceInfoPtr != NULL)
  {
    t_sys_data_current.enable_LOGO_interface = (unsigned char)(*LOGOinterfaceInfoPtr - '0');  //计算保存是否开启开机LOGO界面
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF20_LOGOorNO"); //报错 。没有发现是否开启开机LOGO
    //    sys_add_data(SYS_CONFIG_CF20_LOGOorNO, "0");
    //    is_sys_data_add = true;
  }
}
/********************************************************************
* @brief   : 解析LOGO图片ID
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：John
* @date    ：2017/05/05 PM
* @versions：V1.0
********************************************************************/
void SysConfig::getlogoID(void)
{
  static char *LOGOID;
  LOGOID = find_key_value(SYS_CONFIG_CF21_LOGOID);   //读取 SYS_CONFIG_CF21_LOGOID( "CF21:" ) 在 InfoBuf 中的索引号

  if (LOGOID != NULL)
  {
    t_sys_data_current.logo_id = (unsigned char)strtol(LOGOID, NULL, 10); //计算保存是否开启开机LOGO界面
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF21_LOGOID"); //报错 。没有发现是否开启开机LOGO
    //    sys_add_data(SYS_CONFIG_CF21_LOGOID, "0");
    //    is_sys_data_add = true;
  }
}
/********************************************************************
* @brief   : 获取客户定制机型id
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：John
* @date    ：2017/05/05 PM
* @versions：V1.0
********************************************************************/
void SysConfig::getCustomModelId(void)
{
  static char *CustomModelId;
  CustomModelId = find_key_value(SYS_CONFIG_CF22_CUSTOM_MODEL_ID);   //读取 SYS_CONFIG_CF22_CUSTOM_MODEL_ID( "CF21:" ) 在 InfoBuf 中的索引号

  if (CustomModelId != NULL)
  {
    t_sys_data_current.custom_model_id = atoi(CustomModelId);                                    //字符串转数字
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF22_CUSTOM_MODEL_ID"); //报错 。没有发现是否开启开机LOGO
    //    sys_add_data(SYS_CONFIG_CF22_CUSTOM_MODEL_ID, "0");
    //    is_sys_data_add = true;
  }
}
/********************************************************************
* @brief   : 解析蜂鸣器提示音值
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：John
* @date    ：2017/07/31
* @versions：V1.0
********************************************************************/
void SysConfig::getBezzerSound(void)
{
  char *BuzzerPtr;
  BuzzerPtr = find_key_value(SYS_CONFIG_CF23_CUSTOM_BEZZERSOUND);   //读取 SYS_CONFIG_CF23_CUSTOM_BEZZERSOUND( "CF23:" ) 在 InfoBuf 中的索引号

  if (BuzzerPtr != NULL)
  {
    t_sys_data_current.buzzer_value = (unsigned char)strtol(BuzzerPtr, NULL, 10); //计算保存是否开启开机LOGO界面
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CUSTOM_BEZZERSOUND"); //报错 。没有发现提示音标志
    //    sys_add_data(SYS_CONFIG_CF23_CUSTOM_BEZZERSOUND, "22");
    //    is_sys_data_add = true;
  }
}

void SysConfig::getZOffsetZeroValue(void)
{
  char *zOffsetZeroPtr;
  zOffsetZeroPtr = find_key_value(SYS_CONFIG_CF24_Z_OFFSET_ZERO_VALUE);   //读取 SYS_CONFIG_CF24_Z_OFFSET_ZERO_VALUE( "CF24:" ) 在 InfoBuf 中的索引号

  if (zOffsetZeroPtr != NULL)
  {
    t_sys_data_current.z_offset_value = (float)atof(zOffsetZeroPtr);  //计算保存是否开启开机LOGO界面
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF24_Z_OFFSET_ZERO_VALUE"); //报错 。没有发现提示音标志
    //    sys_add_data(SYS_CONFIG_CF24_Z_OFFSET_ZERO_VALUE, "0.0",true );
    //    is_sys_data_add = true;
  }
}

void SysConfig::getV5ExtruderMask(void)
{
  char *v5ExtruderPtr;
  v5ExtruderPtr = find_key_value(SYS_CONFIG_CF25_V5_EXTRUDER_MASK);   //读取 SYS_CONFIG_CF24_Z_OFFSET_ZERO_VALUE( "CF24:" ) 在 InfoBuf 中的索引号

  if (v5ExtruderPtr != NULL)
  {
    t_sys_data_current.enable_v5_extruder = (unsigned char)(*v5ExtruderPtr - '0');  //计算保存是否开启开机LOGO界面
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF25_V5_EXTRUDER_MASK"); //报错 。没有发现提示音标志
    //    sys_add_data(SYS_CONFIG_CF25_V5_EXTRUDER_MASK, "0");
    //    is_sys_data_add = true;
  }
}

/********************************************************************
* @brief   : 通过从SD卡读取的机型信息编号查找将机型信息编号写进机型数组中
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 PM
* @versions：V1.0
********************************************************************/
void SysConfig::getModelStr(void)
{
  (void)strcpy(t_sys.model_str, model_name_table[t_sys_data_current.model_id]);                 // 通过从SD卡读取的机型信息编号查找将机型信息编号写进 modelStr 数组中
}

/********************************************************************
* @brief   : 将从SD卡读取的功能和版本信息分别写进功能和版本数组中
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 PM
* @versions：V1.0
********************************************************************/
void SysConfig::getStatusInfoStr(void)
{
  //  if (t_sys_data_current.enable_LOGO_interface)
  //    (void)strcat(t_sys.model_str, "+LOGO");                                   //开启了开机LOGO界面，则在机型中显示“LOGO”字样
  /***********根据SD读取信息配置功能信息*********/
  (void)strcpy(t_sys.function_str, "Base"); //Base+CM+POR+MC+LED 基础+混色+断电续打+断料检测+LED照明

  if (t_sys_data_current.enable_color_mixing)
  {
    (void)strcat(t_sys.function_str, "+CM");                                // 混色功能
  }

  if (t_sys_data_current.enable_powerOff_recovery)
  {
    (void)strcat(t_sys.function_str, "+POR");                               // 断电续打功能
  }

  if (t_sys_data_current.enable_material_check)
  {
    (void)strcat(t_sys.function_str, "+MC");                                // 断料检测功能
  }

  if (t_sys_data_current.enable_block_detect)
  {
    (void)strcat(t_sys.function_str, "+BD");
  }

  if (1 == t_sys_data_current.enable_bed_level)
  {
    (void)strcat(t_sys.function_str, "+BL1");
  }

  if (2 == t_sys_data_current.enable_bed_level)
  {
    (void)strcat(t_sys.function_str, "+BL2");
  }

  if (t_sys_data_current.enable_soft_filament)
  {
    (void)strcat(t_sys.function_str, "+SF");                                 //使用了软料齿轮
  }

  if (1 == t_sys_data_current.enable_v5_extruder)
  {
    (void)strcat(t_sys.function_str, "+V5");                                 //使用了软料齿轮
  }

  if (2 == t_sys_data_current.enable_v5_extruder)
  {
    (void)strcat(t_sys.function_str, "+V5.1");                                 //使用了软料齿轮
  }

  if (t_sys_data_current.enable_cavity_temp)
  {
    (void)strcat(t_sys.function_str, "+CT");                                 //腔体温度
  }

  /******************版本信息******************/
  (void)snprintf(t_sys.version_str, sizeof(t_sys.version_str), "D%s-%s-%s", build_date_str, /*app2_ver_str*/APP2_VERSION, boot_ver_str); //D20151110-V3.0.0-V2.0
}

void SysConfig::getCF26(void)
{
  char *cf26;
  cf26 = find_key_value(SYS_CONFIG_CF26_ENABLE_CAVITY_TEMP);

  if (cf26 != NULL)
  {
    t_sys_data_current.enable_cavity_temp = (unsigned char)(*cf26 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF26");
  }
}

void SysConfig::getCF27(void)
{
  char *cf27;
  cf27 = find_key_value(SYS_CONFIG_CF27_TYPE_OF_THERMISTOR);

  if (cf27 != NULL)
  {
    t_sys_data_current.enable_type_of_thermistor = (unsigned char)(*cf27 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF27_TYPE_OF_THERMISTOR");
  }
}

void SysConfig::getCF28(void)
{
  char *cf28;
  cf28 = find_key_value(SYS_CONFIG_CF28_ENABLE_HIGH_TEMP);

  if (cf28 != NULL)
  {
    t_sys_data_current.enable_high_temp = (unsigned char)(*cf28 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF28_ENABLE_HIGH_TEMP");
  }
}

void SysConfig::getCF29(void)
{
  char *cf29;
  cf29 = find_key_value(SYS_CONFIG_CF29_UI_NUMBER);

  if (cf29 != NULL)
  {
    t_sys_data_current.ui_number = (unsigned char)(*cf29 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF29_UI_NUMBER");
  }
}

void SysConfig::getCF30(void)
{
  char *cf30;
  cf30 = find_key_value(SYS_CONFIG_CF30_IS_2GT);

  if (cf30 != NULL)
  {
    t_sys_data_current.is_2GT = (unsigned char)(*cf30 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF30_IS_2GT");
  }
}

void SysConfig::getCF31(void)
{
  char *cf31;
  cf31 = find_key_value(SYS_CONFIG_CF31_IS_MECHANISM_LEVEL);

  if (cf31 != NULL)
  {
    t_sys_data_current.IsMechanismLevel = (unsigned char)(*cf31 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF31");
  }
}

void SysConfig::getCF32(void)
{
  char *cf32;
  cf32 = find_key_value(SYS_CONFIG_CF32_IS_LASER);

  if (cf32 != NULL)
  {
    t_sys_data_current.IsLaser = (unsigned char)(*cf32 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF32");
  }
}

void SysConfig::getCF33(void)
{
  char *cf33;
  cf33 = find_key_value(SYS_CONFIG_CF33_IS_LASER_MODE);

  if (cf33 != NULL)
  {
    t_sys_data_current.IsLaserMode = (unsigned char)(*cf33 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF33");
  }
}

void SysConfig::getCF34(void)
{
  char *cf34;
  cf34 = find_key_value(SYS_CONFIG_CF34);

  if (cf34 != NULL)
  {
    t_sys_data_current.cf34 = (unsigned char)(*cf34 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF34");
  }
}

void SysConfig::getCF35(void)
{
  char *cf35;
  cf35 = find_key_value(SYS_CONFIG_CF35);

  if (cf35 != NULL)
  {
    t_sys_data_current.cf35 = (unsigned char)(*cf35 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF35");
  }
}

void SysConfig::getCF36(void)
{
  char *cf36;
  cf36 = find_key_value(SYS_CONFIG_CF36);

  if (cf36 != NULL)
  {
    t_sys_data_current.cf36 = (unsigned char)(*cf36 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF36");
  }
}

void SysConfig::getCF37(void)
{
  char *cf37;
  cf37 = find_key_value(SYS_CONFIG_CF37);

  if (cf37 != NULL)
  {
    t_sys_data_current.cf37 = (unsigned char)(*cf37 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF37");
  }
}

void SysConfig::getCF38(void)
{
  char *cf38;
  cf38 = find_key_value(SYS_CONFIG_CF38);

  if (cf38 != NULL)
  {
    t_sys_data_current.cf38 = (unsigned char)(*cf38 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF38");
  }
}

void SysConfig::getCF39(void)
{
  char *cf39;
  cf39 = find_key_value(SYS_CONFIG_CF39);

  if (cf39 != NULL)
  {
    t_sys_data_current.cf39 = (unsigned char)(*cf39 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF39");
  }
}

void SysConfig::getCF40(void)
{
  char *cf40;
  cf40 = find_key_value(SYS_CONFIG_CF40);

  if (cf40 != NULL)
  {
    t_sys_data_current.cf40 = (unsigned char)(*cf40 - '0');
  }
  else
  {
    USER_ErrLog("sysconfig no found SYS_CONFIG_CF40");
  }
}

/********************************************************************
* @brief   : 打印系统配置信息 （ 调试使用 ）
* @param   : 无
* @retval  : 无
* @notice  ：无
* @author  ：YK
* @date    ：2016/11/24 PM
* @versions：V1.0
********************************************************************/
void SysConfig::sysconfigLog(void)
{
#if (1 == DEBUG_SYSCONFIG_CUSTOM)
  USER_DbgLog("%40s", "########sysconfig start########");// 是否打开系统信息打印 开始
  USER_DbgLog("%40s = %d", "modelId", t_sys_data_current.model_id);    // 打印机型编号 和 界面语种编号
  USER_DbgLog("%40s = %d", "modelPicId", t_sys_data_current.pic_id);
  USER_DbgLog("%40s = %d", "isEnableColorMixing", t_sys_data_current.enable_color_mixing); // 打印是否使能混色功能
  USER_DbgLog("%40s = %d", "isEnablePowerOffRecovery", t_sys_data_current.enable_powerOff_recovery);     // 打印是否使能断电续打
  USER_DbgLog("%40s = %d", "isEnableMaterialCheck", t_sys_data_current.enable_material_check);        // 打印是否使能断料检测功能
  USER_DbgLog("%40s = %d", "isHaveSetMachine", t_sys_data_current.have_set_machine);             // 打印系统是否已经配置 （决定是否跳出系统配置界面 ）
  USER_DbgLog("%40s = %6.2f", "powerOffRecoveryZMaxValue", t_sys_data_current.poweroff_rec_z_max_value); // 打印断电重启z轴的最大高度
  USER_DbgLog("%40s = %6.2f", "materialCheckVolValue", t_sys_data_current.material_chk_vol_value);     // 打印断料检测值
  USER_DbgLog("%40s = %d", "isEnableBlockDetect", t_sys_data_current.enable_block_detect); // 打印是否使能块检测功能
  USER_DbgLog("%40s = %s", "modelStr", t_sys.model_str);            // 打印机型信息
  USER_DbgLog("%40s = %s", "functionStr", t_sys.function_str);         // 打印功能信息
  USER_DbgLog("%40s = %s", "versionStr", t_sys.version_str);          // 打印版本信息
  USER_DbgLog("%40s = %s", "FWBuildDateBuf", t_sys_data_current.build_date_str);      // 打印固件建立时间信息
  USER_DbgLog("%40s = %s", "FWBootVersionBuf", t_sys_data_current.boot_ver_strot);    // 打印bootload版本信息
  USER_DbgLog("%40s = %s", "FWApp2VersionBuf", t_sys_data_current.app2_ver_str);    // 打印app版本信息
  USER_DbgLog("%40s", "########sysconfig end########");                       // 结束
#endif
}

