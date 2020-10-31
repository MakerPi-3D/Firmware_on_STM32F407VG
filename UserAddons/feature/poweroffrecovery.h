#ifndef POWEROFFRECOVERY_H
#define POWEROFFRECOVERY_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

  extern void poweroff_data_init(void);
  extern void poweroff_start_print_init(uint32_t sd_position_start);
  extern void poweroff_set_data(void);
  extern void poweroff_sync_data(void);
  extern void poweroff_reset_flag(void);
  extern void poweroff_delete_file_from_sd(void);
#ifdef __cplusplus
} //extern "C" {
#endif

/**
 * 断电保存、恢复、校正Z高度操作
 */
class PowerOffOperation
{
public:
  PowerOffOperation();

  void startCalculateZMaxPos(void);                  ///< 校正Z高度
  void getZMaxPos(void);                             ///< 获取Z高度值
  void stopGetZMaxPos(void);                         ///< 获取Z高度值
//  void resetFlag(void);                              ///< 重置断电标志位
//  void setData(void);                                ///< 设置保存数据
  void readyToRecoveryPrint(void);                   ///< 恢复打印前，执行一些准备操作
  void recoveryPrint(void);                          ///< 恢复打印
  void recoveryPrintLoop(void);                      ///< 恢复打印流程
  void recoveryPrintFinish(void);                    ///< 恢复打印
//  void deleteFileFromSD(void);                       ///< 删除SD卡断电文件
private:
  void recoveryTemp(void);                           ///< 恢复打印流程

};

extern PowerOffOperation powerOffOperation;

#endif // POWEROFFRECOVERY_H

