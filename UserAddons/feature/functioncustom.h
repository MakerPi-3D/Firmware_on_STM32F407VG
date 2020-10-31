#ifndef FUNCTIONCUSTOM_H
#define FUNCTIONCUSTOM_H

#ifdef __cplusplus
extern "C" {
#endif

// 断电恢复接口
  extern void poweroff_start_cal_z_max_pos(void);
  extern void poweroff_stop_cal_z_max_pos(void);
  extern void poweroff_ready_to_recover_print(void);

// 测试固件接口
  extern void board_test_display_function(void);
  extern void board_test_model_select(void);
  extern void board_test_cal_heat_time_gui(void);
  extern void board_test_cal_touch_count(void);
  extern void board_test_pressure(void);

#ifdef __cplusplus
} //extern "C" {
#endif

#endif // FUNCTIONCUSTOM_H

