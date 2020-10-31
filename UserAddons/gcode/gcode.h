/**
 * 
 *
 */
#pragma once

#ifndef __GCODE_H
#define __GCODE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  extern void process_commands(void);
  extern void stepper_quick_stop(void); ///< 快速停止运动
  extern bool t_set_targeted_hotend(int16_t code);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif /* __GCODE_H */
