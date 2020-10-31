#ifndef USER_FATFS_H
#define USER_FATFS_H
#ifdef __cplusplus
extern "C" {
#endif

  extern void f_mount_SDCard(void);
  extern void f_mount_Udisk(void);
  extern void f_unmount_Udisk(void);

  extern void UpdateUdiskStatus(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // USER_FATFS_H

