#ifndef GLOBALVARIABLES_CCMRAM_H
#define GLOBALVARIABLES_CCMRAM_H

#include "planner_running_status.h"
#include "planner_block_buffer.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include <stdint.h>
//#include "uart.h"
#ifdef __cplusplus
extern "C" {
#endif

#define PictureFileBufSize (512) // 12k大小
#define POWER_OFF_BUF_SIZE 448
#define BUFSIZE          32          /*!< 定义环形数组大小为 256 */
#define CMD_BUF_SIZE     96          /*!< 定义最大接收字节数 96 */
#define FILE_READ_SIZE  512

  ///////////////////////////////////// PowerOffRecovery Start///////////////////////////////////////////

  typedef struct
  {
    char path_file_name[100];                     // 断电续打文件路徑全名
    char file_name[100];                          // 断电续打文件名
    float feed_rate;                              // 出料速度
    float x_pos;                                  // X轴位置
    float y_pos;                                  // Y轴位置
    float z_pos;                                  // Z轴位置
    float e_pos;                                  // E轴位置
    float b_pos;                                  // B轴位置
    uint32_t sd_pos;                              // 文件指针位置
    uint16_t bed_target_temp;                     // 热床目标温度
    uint16_t nozzle_target_temp;                  // 喷嘴目标温度
    uint16_t fan_speed;                           // 风扇速度
    uint16_t feed_multiply;                       // 打印速度
    uint8_t flag;                                 // 是否需要断电续打
    uint8_t is_file_from_sd;                      // 断电续打文件是否在SD卡中
    uint8_t is_power_off;                         // 是否已经断电
    uint8_t blockdetectflag;                      // 标志堵料
    uint8_t serial_flag;
  } T_POWER_OFF;
///////////////////////////////////// PowerOffRecovery End///////////////////////////////////////////

///////////////////////////////////// CustomServices Start///////////////////////////////////////////
#define CAL_Z_MAX_POS_OFFSET 20.0F                 // 校準Z最大位置時，校準值與原來MAX Z的偏移量
  typedef struct
  {
    uint8_t disable_abs;                          // 是否能打印ABS
    uint8_t disable_hot_bed;                      // 是否开启热床
    uint8_t enable_warning_light;                 // 是否有警示灯
    uint8_t enable_led_light;                     // 是否有LED照明
  } T_CUSTOM_SERVICES;

///////////////////////////////////// CustomServices End///////////////////////////////////////////

///////////////////////////////////// GUI Start///////////////////////////////////////////
  typedef struct
  {
    int32_t printed_time_sec;                         // 已打印的时间
    int32_t used_total_material;                      // 耗材总长度
    long machine_run_time;                            // 机器运行时间
    int16_t nozzle_temp;                              // 喷嘴温度
    int16_t target_nozzle_temp;                       // 喷嘴目标温度
    int16_t hot_bed_temp;                             // 热床温度
    int16_t target_hot_bed_temp;                      // 热床目标温度
    uint16_t print_speed_value;                       // 打印速度
    uint16_t fan_speed_value;                         // 风扇速度
    uint16_t cavity_temp;                             //
    uint16_t target_cavity_temp;                      //
    uint16_t target_cavity_temp_on;                   //
    int32_t move_xyz_pos[3];                          // 移动XYZ轴
    uint8_t print_percent;                            // 打印进度-百分数
    uint32_t printfile_size;                          // 打印文件总大小（字节）
    uint32_t file_size;                               // 打印文件剩余大小（字节）
    uint32_t file_position;                           // 打印文件当前指针位置
    uint16_t cura_speed;                              // 获取到的cura软件上的速度，M117命令传送
  } T_GUI;
///////////////////////////////////// GUI End///////////////////////////////////////////

  extern char PictureFileBuf[PictureFileBufSize];                    /*!< 处于IRAM2区的起始地址 */
  extern unsigned short NozzleTempTextRangeBuf[24*12*3];             /*!< 喷嘴温度显示区域数组，24高12宽3个英文字符 */
  extern unsigned short HotBedTempTextRangeBuf[24*12*3];             /*!< 热床温度显示区域数组，24高12宽3个英文字符 */
  extern unsigned short NozzleTargetTempTextRangeBuf[24*12*3];       /*!< 目标喷嘴温度显示区域数组，24高12宽3个英文字符 */
  extern unsigned short HotBedTargetTempTextRangeBuf[24*12*3];       /*!< 目标热床温度显示区域数组，24高12宽3个英文字符 */
  extern unsigned short PrintScheduleShape[24*12*3];                 /*!< 打印进度显示区域数组，24高12宽3个英文字符 */
  extern unsigned short CavityTempTextRangeBuf[24*12*3];             /*!< 腔体温度显示区域数组，24高12宽3个英文字符 */
  extern unsigned short CavityTargetTempTextRangeBuf[24*12*3];       /*!< 目标腔体温度显示区域数组，24高12宽3个英文字符 */
  extern unsigned short TimeTextRangeBuf[24*12*11];                  /*!< 时间显示区域数组，24高12宽9个英文字符 */
  extern unsigned short TextRangeBuf[24*12*11];                      /*!< 数值显示合成区域数组，24高12宽9个英文字符 */
  extern unsigned short SpeedRangeBuf[24*12*9];                      /*!< 打印速度显示区域数组，24高12宽9个英文字符 */
  extern unsigned short XPosRangeBuf[24*12*3];                       /*!< X位置显示区域数组，24高12宽3个英文字符 */
  extern unsigned short YPosRangeBuf[24*12*3];                       /*!< Y位置显示区域数组，24高12宽3个英文字符 */
  extern unsigned short ZPosRangeBuf[24*12*3];                       /*!< Z位置显示区域数组，24高12宽3个英文字符 */
  extern unsigned short PrintSpeedRangeBuf[24*12*3];                 /*!< 打印速度显示区域数组，24高12宽3个英文字符 */
  extern unsigned short FanSpeedRangeBuf[24*12*3];                   /*!< 风扇速度显示区域数组，24高12宽3个英文字符 */
	extern unsigned short LedSwitchRangeBuf[24*12*3];                  /*!< led灯显示区域数组，24高12宽3个英文字符 */
  extern unsigned short CavityTempOnRangeBuf[24*12*3];               /*!< 腔体开关显示区域数组，24高12宽3个英文字符 */
  extern char poweroff_data[POWER_OFF_BUF_SIZE];                     /*!< 斷電續打數據緩存 */
  extern volatile planner_running_status_t po_save_running_status[4];         /*!< 断电续打保存参数 */
  extern volatile planner_running_status_t runningStatus[BLOCK_BUFFER_SIZE];  /*!< 运动参数数组 */
  extern volatile planner_running_status_t running_status;                    /*!< 当前运动参数 */
  extern volatile block_t block_buffer[BLOCK_BUFFER_SIZE];                    /*!< A ring buffer for motion instfructions */
  extern volatile unsigned char block_buffer_head;                   /*!< Index of the next block to be pushed */
  extern volatile unsigned char block_buffer_tail;                   /*!< Index of the block to process now */
  extern volatile char command_buffer[BUFSIZE][CMD_BUF_SIZE];                 /*!< 环形指令队列 */
  extern volatile uint32_t command_buffer_head;                      /*!< 环形指令队列头 */
  extern volatile uint32_t command_buffer_tail;                      /*!< 环形指令队列尾 */
  extern char file_read_buf[FILE_READ_SIZE];                         /*!< 读取文件数组对象 */
  extern char sys_data[512];                                         /*!< sysconfig數據緩存 */
  extern T_SYS t_sys;                                                /*!<  */
  extern volatile T_SYS_DATA t_sys_data_current;                     /*!<  */
  extern volatile motion_3d_t motion_3d;                             /*!<  */
  extern volatile motion_3d_model_t motion_3d_model;                 /*!<  */
  extern volatile T_CUSTOM_SERVICES t_custom_services;               /*!<  */
  extern volatile T_GUI t_gui;                                       /*!<  */
  extern T_POWER_OFF t_power_off;                                    /*!<  */
  extern volatile int file_read_buf_index ;                          /*!<  */

  extern void ccmram_data_init(void);
#ifdef __cplusplus
} //extern "C"
#endif

#endif // GLOBALVARIABLES_CCMRAM_H

