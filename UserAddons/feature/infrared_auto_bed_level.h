#ifndef INFRARED_AUTO_BED_LEVEL_H
#define INFRARED_AUTO_BED_LEVEL_H
#include "wbtypes.h"
#ifdef __cplusplus
extern "C" {
#endif

  void infrared_auto_bed_init_position(CONST FLOAT &_left_bed_position, CONST FLOAT &_right_bed_position, CONST FLOAT &_front_bed_position, CONST FLOAT &_back_bed_position, CONST FLOAT &_middle_bed_position);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // INFRARED_AUTO_BED_LEVEL_H

