#include "globalvariables_ccmram.h"

#define CCMRAM_BASE_ADD (0X1000FFFC)
#define CCMRAM_PIC_BUF_ADD (CCMRAM_BASE_ADD - (sizeof(char)* PictureFileBufSize))
#define CCMRAM_NOZ_TEMP_ADD (CCMRAM_PIC_BUF_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_HOTBED_TEMP_ADD (CCMRAM_NOZ_TEMP_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_NOZ_TARG_TEMP_ADD (CCMRAM_HOTBED_TEMP_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_HOTBED_TARG_TEMP_ADD (CCMRAM_NOZ_TARG_TEMP_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_PRINT_SCHEDULE_ADD (CCMRAM_HOTBED_TARG_TEMP_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_CAVITY_TEMP_ADD (CCMRAM_PRINT_SCHEDULE_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_CAVITY_TARG_TEMP_ADD (CCMRAM_CAVITY_TEMP_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_TIME_TEXT_TEMP_ADD (CCMRAM_CAVITY_TARG_TEMP_ADD - (sizeof(unsigned short)*24*12*11))
#define CCMRAM_TEXT_ADD (CCMRAM_TIME_TEXT_TEMP_ADD - (sizeof(unsigned short)*24*12*11))
#define CCMRAM_SPEED_ADD (CCMRAM_TEXT_ADD - (sizeof(unsigned short)*24*12*9))
#define CCMRAM_X_POS_ADD (CCMRAM_SPEED_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_Y_POS_ADD (CCMRAM_X_POS_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_Z_POS_ADD (CCMRAM_Y_POS_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_PRINT_SPEED_ADD (CCMRAM_Z_POS_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_FAN_SPEED_ADD (CCMRAM_PRINT_SPEED_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_LED_SWITCH_ADD (CCMRAM_FAN_SPEED_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_CAVITY_SWITCH_ADD (CCMRAM_LED_SWITCH_ADD - (sizeof(unsigned short)*24*12*3))
#define CCMRAM_POWER_DATA_ADD (CCMRAM_CAVITY_SWITCH_ADD - (sizeof(char)*POWER_OFF_BUF_SIZE))
#define CCMRAM_POWER_SAVE_RUN_ADD (CCMRAM_POWER_DATA_ADD - (sizeof(planner_running_status_t)*4))
#define CCMRAM_RUN_STATUSES_ADD (CCMRAM_POWER_SAVE_RUN_ADD - (sizeof(planner_running_status_t)*BLOCK_BUFFER_SIZE))
#define CCMRAM_RUN_STATUSE_CURR_ADD (CCMRAM_RUN_STATUSES_ADD - (sizeof(planner_running_status_t)))
#define CCMRAM_BLOCK_BUF_ADD (CCMRAM_RUN_STATUSE_CURR_ADD - (sizeof(block_t)*BLOCK_BUFFER_SIZE))
#define CCMRAM_BLOCK_BUF_HEAD_ADD (CCMRAM_BLOCK_BUF_ADD - (sizeof(unsigned char)*4))
#define CCMRAM_BLOCK_BUF_TAIL_ADD (CCMRAM_BLOCK_BUF_HEAD_ADD - (sizeof(unsigned char)*4))
#define CCMRAM_CMD_BUF_ADD (CCMRAM_BLOCK_BUF_TAIL_ADD - (sizeof(char)*BUFSIZE*CMD_BUF_SIZE))
#define CCMRAM_CMD_BUF_HEAD_ADD (CCMRAM_CMD_BUF_ADD - (sizeof(uint32_t)))
#define CCMRAM_CMD_BUF_TAIL_ADD (CCMRAM_CMD_BUF_HEAD_ADD - (sizeof(uint32_t)))
#define CCMRAM_FILE_READ_BUF_ADD (CCMRAM_CMD_BUF_TAIL_ADD - (sizeof(char)*FILE_READ_SIZE))
#define CCMRAM_SYS_DATA_ADD (CCMRAM_FILE_READ_BUF_ADD - (sizeof(char)*512))
#define CCMRAM_T_SYS_ADD (CCMRAM_SYS_DATA_ADD - (sizeof(T_SYS)))
#define CCMRAM_T_SYS_DATA_ADD (CCMRAM_T_SYS_ADD - (sizeof(T_SYS_DATA)))
#define CCMRAM_MOTION_3D_ADD (CCMRAM_T_SYS_DATA_ADD - (sizeof(motion_3d_t)))
#define CCMRAM_MOTION_3D_MODEL_ADD (CCMRAM_MOTION_3D_ADD - (sizeof(motion_3d_model_t)))
#define CCMRAM_T_CUSTOM_SERVICES_ADD (CCMRAM_MOTION_3D_MODEL_ADD - (sizeof(T_CUSTOM_SERVICES)))
#define CCMRAM_T_GUI_ADD (CCMRAM_T_CUSTOM_SERVICES_ADD - (sizeof(T_GUI)))
#define CCMRAM_T_POWER_OFF_ADD (CCMRAM_T_GUI_ADD - (sizeof(T_POWER_OFF)))
#define CCMRAM_FILE_READ_BUF_INDEX_ADD (CCMRAM_T_POWER_OFF_ADD - (sizeof(int)))

//IRAM2共64K，起始地址0x10000000  IRAM1共128K，起始地址0x20000000
char PictureFileBuf[PictureFileBufSize]                    __attribute__((at(CCMRAM_PIC_BUF_ADD)));          /*!< 处于IRAM2区的起始地址 */
unsigned short NozzleTempTextRangeBuf[24*12*3]             __attribute__((at(CCMRAM_NOZ_TEMP_ADD)));         /*!< 喷嘴温度显示区域数组，24高12宽3个英文字符 */
unsigned short HotBedTempTextRangeBuf[24*12*3]             __attribute__((at(CCMRAM_HOTBED_TEMP_ADD)));      /*!< 热床温度显示区域数组，24高12宽3个英文字符 */
unsigned short NozzleTargetTempTextRangeBuf[24*12*3]       __attribute__((at(CCMRAM_NOZ_TARG_TEMP_ADD)));    /*!< 目标喷嘴温度显示区域数组，24高12宽3个英文字符 */
unsigned short HotBedTargetTempTextRangeBuf[24*12*3]       __attribute__((at(CCMRAM_HOTBED_TARG_TEMP_ADD))); /*!< 目标热床温度显示区域数组，24高12宽3个英文字符 */
unsigned short PrintScheduleShape[24*12*3]                 __attribute__((at(CCMRAM_PRINT_SCHEDULE_ADD)));   /*!< 打印进度显示区域数组，24高12宽3个英文字符 */
unsigned short CavityTempTextRangeBuf[24*12*3]             __attribute__((at(CCMRAM_CAVITY_TEMP_ADD)));      /*!< 腔体温度显示区域数组，24高12宽3个英文字符 */
unsigned short CavityTargetTempTextRangeBuf[24*12*3]       __attribute__((at(CCMRAM_CAVITY_TARG_TEMP_ADD))); /*!< 目标腔体温度显示区域数组，24高12宽3个英文字符 */
unsigned short TimeTextRangeBuf[24*12*11]                  __attribute__((at(CCMRAM_TIME_TEXT_TEMP_ADD)));   /*!< 时间显示区域数组，24高12宽11个英文字符 */
unsigned short TextRangeBuf[24*12*11]                      __attribute__((at(CCMRAM_TEXT_ADD)));             /*!< 数值显示合成区域数组，24高12宽11个英文字符 */
unsigned short SpeedRangeBuf[24*12*9]                      __attribute__((at(CCMRAM_SPEED_ADD)));            /*!< 打印速度显示区域数组，24高12宽9个英文字符 */
unsigned short XPosRangeBuf[24*12*3]                       __attribute__((at(CCMRAM_X_POS_ADD)));            /*!< X位置显示区域数组，24高12宽3个英文字符 */
unsigned short YPosRangeBuf[24*12*3]                       __attribute__((at(CCMRAM_Y_POS_ADD)));            /*!< Y位置显示区域数组，24高12宽3个英文字符 */
unsigned short ZPosRangeBuf[24*12*3]                       __attribute__((at(CCMRAM_Z_POS_ADD)));            /*!< Z位置显示区域数组，24高12宽3个英文字符 */
unsigned short PrintSpeedRangeBuf[24*12*3]                 __attribute__((at(CCMRAM_PRINT_SPEED_ADD)));      /*!< 打印速度显示区域数组，24高12宽3个英文字符 */
unsigned short FanSpeedRangeBuf[24*12*3]                   __attribute__((at(CCMRAM_FAN_SPEED_ADD)));        /*!< 风扇速度显示区域数组，24高12宽3个英文字符 */
unsigned short LedSwitchRangeBuf[24*12*3]                  __attribute__((at(CCMRAM_LED_SWITCH_ADD)));       /*!< led灯显示区域数组，24高12宽3个英文字符 */
unsigned short CavityTempOnRangeBuf[24*12*3]               __attribute__((at(CCMRAM_CAVITY_SWITCH_ADD)));    /*!< 腔体开关显示区域数组，24高12宽3个英文字符 */
char poweroff_data[POWER_OFF_BUF_SIZE]                     __attribute__((at(CCMRAM_POWER_DATA_ADD)));       /*!< 斷電續打數據緩存 */
volatile planner_running_status_t po_save_running_status[4]         __attribute__((at(CCMRAM_POWER_SAVE_RUN_ADD)));   /*!< 断电续打保存参数 */
volatile planner_running_status_t runningStatus[BLOCK_BUFFER_SIZE]  __attribute__((at(CCMRAM_RUN_STATUSES_ADD)));     /*!< 运动参数数组 */
volatile planner_running_status_t running_status                    __attribute__((at(CCMRAM_RUN_STATUSE_CURR_ADD))); /*!< 当前运动参数 */
volatile block_t block_buffer[BLOCK_BUFFER_SIZE]                    __attribute__((at(CCMRAM_BLOCK_BUF_ADD)));        /*!< A ring buffer for motion instfructions */
volatile unsigned char block_buffer_head                   __attribute__((at(CCMRAM_BLOCK_BUF_HEAD_ADD)));   /*!< Index of the next block to be pushed */
volatile unsigned char block_buffer_tail                   __attribute__((at(CCMRAM_BLOCK_BUF_TAIL_ADD)));   /*!< Index of the block to process now */
volatile char command_buffer[BUFSIZE][CMD_BUF_SIZE]                 __attribute__((at(CCMRAM_CMD_BUF_ADD)));          /*!< 环形指令队列 */
volatile uint32_t command_buffer_head                      __attribute__((at(CCMRAM_CMD_BUF_HEAD_ADD)));     /*!< 环形指令队列头 */
volatile uint32_t command_buffer_tail                      __attribute__((at(CCMRAM_CMD_BUF_TAIL_ADD)));     /*!< 环形指令队列尾 */
char file_read_buf[FILE_READ_SIZE]                         __attribute__((at(CCMRAM_FILE_READ_BUF_ADD)));    /*!< 读取文件数组对象 */
char sys_data[512]                                         __attribute__((at(CCMRAM_SYS_DATA_ADD)));         /*!< sysconfig數據緩存 */
T_SYS t_sys                                                __attribute__((at(CCMRAM_T_SYS_ADD)));            /*!<  */
volatile T_SYS_DATA t_sys_data_current                     __attribute__((at(CCMRAM_T_SYS_DATA_ADD)));       /*!<  */
volatile motion_3d_t motion_3d                             __attribute__((at(CCMRAM_MOTION_3D_ADD)));        /*!<  */
volatile motion_3d_model_t motion_3d_model                 __attribute__((at(CCMRAM_MOTION_3D_MODEL_ADD)));  /*!<  */
volatile T_CUSTOM_SERVICES t_custom_services               __attribute__((at(CCMRAM_T_CUSTOM_SERVICES_ADD)));/*!<  */
volatile T_GUI t_gui                                       __attribute__((at(CCMRAM_T_GUI_ADD)));            /*!<  */
T_POWER_OFF t_power_off                                    __attribute__((at(CCMRAM_T_POWER_OFF_ADD)));      /*!<  */
volatile int file_read_buf_index                           __attribute__((at(CCMRAM_FILE_READ_BUF_INDEX_ADD)));/*!<  */

static void ccmram_t_sys_data_current_init(void)
{
  t_sys_data_current.have_set_machine = 0;                     // 是否已經設置機器
  t_sys_data_current.model_id = 0;                             // 要改变为哪种机型
  t_sys_data_current.pic_id = 0;                               // 要改变为哪种图片 中文图片或日文图片
  t_sys_data_current.enable_powerOff_recovery = 0;             // 是否有斷電功能
  t_sys_data_current.poweroff_rec_z_max_value = 0;             // 斷電z最大高度
  t_sys_data_current.enable_color_mixing = 0;                  // 是否有混色功能
  t_sys_data_current.enable_material_check = 0;                // 是否有断料检测功能
  t_sys_data_current.material_chk_vol_value = 0.0f;            //
  t_sys_data_current.enable_block_detect = 0;                  // 是否開啓堵料檢測
  t_sys_data_current.bed_level_z_at_left_front = 0.0f;         //
  t_sys_data_current.bed_level_z_at_right_front = 0.0f;        //
  t_sys_data_current.bed_level_z_at_left_back = 0.0f;          //
  t_sys_data_current.bed_level_z_at_right_back = 0.0f;         //
  t_sys_data_current.bed_level_z_at_middle = 0.0f;             //
  t_sys_data_current.pid_output_factor = 0.0f;                 // 加热系数
  t_sys_data_current.enable_bed_level = 0;                     // 是否开启自动调平
  t_sys_data_current.enable_soft_filament = 0;                 // 是否使用软料
  t_sys_data_current.enable_LOGO_interface = 0;                // 是否开启开机LOGO界面
  t_sys_data_current.logo_id = 0;                              // logo图片的id号
  t_sys_data_current.custom_model_id = 0;                      // 定制机型id
  t_sys_data_current.buzzer_value = 0;                         // 按键声、报警声开关
  t_sys_data_current.z_offset_value = 0.0f;                    // Z零点偏移
  t_sys_data_current.enable_v5_extruder = 0;
  t_sys_data_current.enable_cavity_temp = 0;
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
}

static void ccmram_motion_3d_init(void)
{
  motion_3d.axis_num = 0;                              // 機型軸數
  motion_3d.step = 0;                            // 機型細分數 1:16;2:32;
  motion_3d.enable_poweroff_up_down_min_min = 0; // 是否開啓上下共限位
  motion_3d.enable_board_test = 0;               // 是否開啓大板測試
  motion_3d.enable_check_door_open = 0;          // 是否有门检测
  motion_3d.disable_z_max_limit = 0;             // 是否有Z轴下限位开关
  motion_3d.updown_g28_first_time = 0;           // 上下限位公用限位開關，執行G28指令判斷Z零點位置
  motion_3d.is_open_infrared_z_min_check = 0;
  motion_3d.is_cal_bed_platform = false;
  motion_3d.extrude_min_temp = 0.0f;
}

static void ccmram_motion_3d_model_init(void)
{
  for(int i = 0; i < 3; i++)
  {
    motion_3d_model.xyz_max_pos[i] = 0.0f;                // XYZ最大位置
    motion_3d_model.xyz_min_pos[i] = 0.0f;                // XYZ最小位置
    motion_3d_model.xyz_home_pos[i] = 0.0f;               // XYZ零點位置
    motion_3d_model.xyz_max_length[i] = 0.0f;             // XYZ最大長度
    motion_3d_model.xyz_home_retract_mm[i] = 0.0f;        // XYZ歸零回抽距離mm
    motion_3d_model.xyz_move_max_pos[i] = 0.0f;           // XYZ最大移动位置
    motion_3d_model.xyz_home_dir[i] = 1;         // XYZ方向
  }
  motion_3d_model.extrude_maxlength = 0.0f;             // prevent extrusion of very large distances.
  motion_3d_model.z_max_pos_origin = 0.0f;              // 保存机型默认Z最大点，用于校准Z高度
  for(int i = 0; i < 6; i++)
  {
    motion_3d_model.enable_invert_dir[i] = 0;  // 是否反轉軸方向
  }
}

static void ccmram_t_custom_services_init(void)
{
  t_custom_services.disable_abs = 0;
  t_custom_services.disable_hot_bed = 0;
  t_custom_services.enable_led_light = 0;
  t_custom_services.enable_warning_light = 0;
}

static void ccmram_t_gui_init(void)
{
  t_gui.printed_time_sec = 0;                         // 已打印的时间
  t_gui.used_total_material = 0;                      // 耗材总长度
  t_gui.machine_run_time = 0;                         // 机器运行时间
  t_gui.nozzle_temp = 0;                              // 喷嘴温度
  t_gui.target_nozzle_temp = 0;                       // 喷嘴目标温度
  t_gui.hot_bed_temp = 0;                             // 热床温度
  t_gui.target_hot_bed_temp = 0;                      // 热床目标温度
  t_gui.print_speed_value = 0;                        // 打印速度
  t_gui.fan_speed_value = 0;                          // 风扇速度
  t_gui.cavity_temp = 0;                              //
  t_gui.target_cavity_temp = 0;                       //
  t_gui.target_cavity_temp_on = 0;                    //
  for(int i = 0; i < 3; i++)
  {
    t_gui.move_xyz_pos[i] = 0;                        // 移动XYZ轴
  }
  t_gui.print_percent = 0;                            // 打印进度-百分数
  t_gui.printfile_size = 0;                           // 打印文件总大小（字节）
  t_gui.file_position = 0;                            // 打印文件当前指针位置
  t_gui.file_size = 0;                                // 打印文件剩余大小（字节）
  t_gui.cura_speed = 0;                               // 获取到的cura软件上的速度，M117命令传送
}

void reset_run_status(volatile planner_running_status_t * const source)
{
  for(int i = 0; i < MAX_NUM_AXIS; i++)
  {
    source->axis_position[i] = 0.0f;
  }
  source->bed_temp = 0;
  source->extruder0_temp = 0;
  source->feed_rate = 0;
  source->sd_position = 0;
  source->extruder_multiply = 0;
  source->feed_multiply = 0;
  source->fan_speed = 0;
  source->extruder = 0;
  source->is_serial = 0;
}

void reset_block(volatile block_t * const source)
{
  for(int i = 0; i < MAX_NUM_AXIS; i++)
  {
    source->steps_axis[i] = 0.0f;
  }
  source->step_event_count = 0;
  source->accelerate_until = 0;
  source->decelerate_after = 0;
  source->acceleration_rate = 0;
  source->direction_bits = 0;
  source->active_extruder = 0;
  source->nominal_speed = 0;
  source->entry_speed = 0;
  source->max_entry_speed = 0;
  source->millimeters = 0;
  source->acceleration = 0;
  source->recalculate_flag = 0;
  source->nominal_length_flag = 0;
  source->busy = 0;
  source->nominal_rate = 0;
  source->initial_rate = 0;
  source->final_rate = 0;
  source->acceleration_st = 0;
}


void ccmram_data_init(void)
{
  memset(PictureFileBuf, 0, sizeof(char)*PictureFileBufSize);
  memset(NozzleTempTextRangeBuf, 0, sizeof(unsigned short)*24*12*3);
  memset(HotBedTempTextRangeBuf, 0, sizeof(unsigned short)*24*12*3);
  memset(NozzleTargetTempTextRangeBuf, 0, sizeof(unsigned short)*24*12*3);
  memset(HotBedTargetTempTextRangeBuf, 0, sizeof(unsigned short)*24*12*3);
  memset(PrintScheduleShape, 0, sizeof(unsigned short)*24*12*3);
  memset(CavityTempTextRangeBuf, 0, sizeof(unsigned short)*24*12*3);
  memset(CavityTargetTempTextRangeBuf, 0, sizeof(unsigned short)*24*12*3);
  memset(TimeTextRangeBuf, 0, sizeof(unsigned short)*24*12*11);
  memset(TextRangeBuf, 0, sizeof(unsigned short)*24*12*11);
  memset(SpeedRangeBuf, 0, sizeof(unsigned short)*24*12*9);
  memset(poweroff_data, 0, sizeof(char)*POWER_OFF_BUF_SIZE);
  for(int i = 0; i < 4; i++)
  {
    reset_run_status(&po_save_running_status[i]);
  }
  for(int i = 0; i < BLOCK_BUFFER_SIZE; i++)
  {
    reset_run_status(&runningStatus[i]);
  }
  reset_run_status(&running_status);
  for(int i = 0; i < BLOCK_BUFFER_SIZE; i++)
  {
    reset_block(&block_buffer[i]);
  }
  block_buffer_head = 0U;
  block_buffer_tail = 0U;
  memset(command_buffer, 0, sizeof(char)*BUFSIZE*CMD_BUF_SIZE);
  command_buffer_head = 1U;
  command_buffer_tail = 0U;
  memset(file_read_buf, 0, sizeof(char)*FILE_READ_SIZE);
  memset(sys_data, 0, sizeof(char)*512);
  memset(&t_sys, 0, sizeof(T_SYS));
  ccmram_t_sys_data_current_init();
  ccmram_motion_3d_init();
  ccmram_motion_3d_model_init();
  ccmram_t_custom_services_init();
  ccmram_t_gui_init();
  memset(&t_power_off, 0, sizeof(T_POWER_OFF));
  file_read_buf_index = FILE_READ_SIZE;
}

