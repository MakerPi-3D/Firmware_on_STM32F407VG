#ifndef cal_z_offset_to_zero_h
#define cal_z_offset_to_zero_h
#ifdef __cplusplus
extern "C" {
#endif
#ifdef CAL_Z_ZERO_OFFSET

  extern void infrared_z_zero_adjust_interface(void);
  extern void infrared_z_zero_adjust_autohome(void);
  extern void infrared_adjust_init(void);

#endif //  CAL_Z_ZERO_OFFSET

#ifdef __cplusplus
} //extern "C" {
#endif
#endif




