#ifndef INFRARED_BED_LEVEL_ADJUST_H
#define INFRARED_BED_LEVEL_ADJUST_H

#ifdef __cplusplus
extern "C" {
#endif
#ifdef CAL_Z_ZERO_OFFSET
  extern void infrared_bed_level_adjust_init(void);
  extern void infrared_bed_level_adjust_start(void);
  extern void infrared_bed_level_adjust_interface(void);
  bool infrared_bed_level_mark(void);
#endif
#ifdef __cplusplus
} // extern "C" {
#endif

#endif

