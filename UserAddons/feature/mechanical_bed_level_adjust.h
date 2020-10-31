#ifndef MECHANICAL_BED_LEVEL_ADJUST_H
#define MECHANICAL_BED_LEVEL_ADJUST_H
#include "stdbool.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef CAL_Z_ZERO_OFFSET
  extern void mechanical_bed_level_adjust_init(void);
  extern void mechanical_bed_level_adjust_start(void);
  extern void mechanical_bed_level_adjust_interface(void);
  extern bool mechanical_bed_level_mark(void);
  extern void Level_Before_Warn(void);
#endif
#ifdef __cplusplus
} // extern "C" {
#endif

#endif



