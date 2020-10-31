#ifndef AUTOBEDLEVELINTERFACE_H
#define AUTOBEDLEVELINTERFACE_H
#define ENABLE_AUTO_BED_LEVELING

#include <stdint.h>

#ifdef ENABLE_AUTO_BED_LEVELING

//#define ENABLE_AUTO_BED_LEVELING_DEBUG //自动调平开关测试模式
#define Z_RAISE_BEFORE_PROBING  (20.0F)
#define Z_RAISE_BETWEEN_PROBINGS (20.0F)
#define Z_MOVEDOWN_UNTIL_FIND_BED (-20.0F)
#define XY_TRAVEL_SPEED (3000.0F)
#define Z_PROBE_OFFSET_FROM_EXTRUDER (0.0F)

#define AUTO_BED_LEVELING_EXTRUDE_MINTEMP (180.0F)
#define AUTO_BED_LEVELING_EXTRUDE_HEATTEMP (210.0F)
#endif // ENABLE_AUTO_BED_LEVELING

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_AUTO_BED_LEVELING
  void auto_bed_level_adjust_init(void);
  void start_calculate_bed_level(void);
  void get_bed_level_position_z_at_lf(void);
  void get_bed_level_position_z_at_lb(void);
  void get_bed_level_position_z_at_middle(void);
  void get_bed_level_position_z_at_rb(void);
  void get_bed_level_position_z_at_rf(void);
  void calculate_bed_level_finish(void);
  bool is_cal_bed_platform(void);
  bool is_auto_bed_level_zero(void);
  void gcodeCMD_g29_interface(bool isSerial);
  void gcodeCMD_g28_bed_level_start(int tmp_extruder);
  void gcodeCMD_g28_bed_level_end(void);
  void gcodeCMD_g28_interface(uint8_t active_extruder);
  void gcodeCMD_m602_bed_level_interface(uint8_t active_extruder);
#else
  inline void auto_bed_level_adjust_init(void) {}
  inline void start_calculate_bed_level(void) {}
  inline void get_bed_level_position_z_at_lf(void) {}
  inline void get_bed_level_position_z_at_lb(void) {}
  inline void get_bed_level_position_z_at_middle(void) {}
  inline void get_bed_level_position_z_at_rb(void) {}
  inline void get_bed_level_position_z_at_rf(void) {}
  inline void calculate_bed_level_finish(void) {}
  inline bool is_cal_bed_platform(void)
  {
    return true;
  }
  inline bool is_auto_bed_level_zero(void) {}
  inline void gcodeCMD_g29_interface(bool isSerial) {}
  inline void gcodeCMD_g28_bed_level_start(INT tmp_extruder) {}
  inline void gcodeCMD_g28_bed_level_end(void) {}
  inline void gcodeCMD_g28_interface(UINT8 active_extruder) {}
  inline void gcodeCMD_m602_bed_level_interface(UINT8 active_extruder) {}
#endif

#ifdef __cplusplus
} //extern "C" {
#endif

#endif // AUTOBEDLEVELINTERFACE_H


