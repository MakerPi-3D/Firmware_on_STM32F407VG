#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdbool.h>
#include "sys_function.h"

#ifdef __cplusplus
extern "C" {
#endif


  extern void main_user_core1_init(void);
  extern void main_user_core2_init(void);

  extern void sys_send_gcode_cmd(const char* gcode_cmd);
  extern bool sys_receive_gcode_cmd(void);
  extern void sys_clear_gcode_cmd(void);

  extern void sys_delay(const int tick_value);

  extern void sys_time2_start(void);
  extern void sys_time5_write_fan_pwm(int pwm_value);

  extern void task_read_udisk_wait(void);
  extern void task_read_udisk_release(void);
  extern void task_receive_uart_wait(void);
  extern void task_receive_uart_release(void);
  extern void task_gui_wait_release(void);
  extern bool task_is_gui_send_wait_done(void);

  extern void gpio_color_mix_control(bool on_or_off);
  extern void gpio_light_control(bool is_on);
  extern void gpio_infrared_level_init(void);
  extern bool gpio_infrared_level_detection(void);
  extern bool gpio_mechanical_level_detection(void);
//  extern void gpio_motor_fan_control(bool is_on);
  extern void gpio_block_detect_init(void);
  extern bool gpio_block_detect_is_a_detection(void);
  extern bool gpio_block_detect_is_b_detection(void);
  extern void gpio_material_check_init(void);
  extern void gpio_material_check_adc_init(void);
  extern void gpio_material_chk_adc_start(void);
  extern uint32_t gpio_material_chk_adc_get_value(void);
  extern void gpio_beep_control(bool is_on);
  extern void gpio_e_motor_fan_control(bool is_on);
  extern void gpio_board_fan_control(bool is_on);
  extern void gpio_infrated_bed_level_init(bool isDeInit);
  extern bool gpio_is_zmin_pin_exit(void);
  extern bool gpio_is_temp0_pin_exit(void);
  extern bool gpio_is_temp1_pin_exit(void);
  extern bool gpio_is_bedtemp_pin_exit(void);



#ifdef __cplusplus
} //extern "C"
#endif

#endif //USER_INTERFACE_H

