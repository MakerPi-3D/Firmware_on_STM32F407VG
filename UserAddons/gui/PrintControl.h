#ifndef PRINTCONTROL_H
#define PRINTCONTROL_H
#include "wbtypes.h"
#ifdef __cplusplus
extern "C" {
#endif

  INT IsPrint(void);                        ///< 是否为打印状态
  INT IsHeating(void);
  void readGcodeBuf(void);
  INT IsPausePrint(void);                   ///< 是否为暂停打印状态
  void SetPausePrintingStatus(bool status); ///< 设置暂停状态
  void SetPrintStatus(bool status);         ///< 设置打印状态
  bool IsFinishedPrint(void);               ///< 是否已经打印完成
  bool IsPrintSDFile(void);                 ///< 是否打印sd卡文件
  void PowerOffRecStartPrint(void);
  bool IsPauseToCoolDown(void);
  bool isProcessPausePrintDone(void);
#ifdef __cplusplus
} //extern "C" {
#endif

/**
 * 打印控制
 */
class PrintControl
{
public:
  PrintControl();
  void stop(bool isSerial);                      ///< 停止打印入口
  void start(bool isSerial);                     ///< 开始打印入口
  void pause(bool isSerial);                     ///< 暂停打印入口
  void resume(bool isSerial);                    ///< 恢复打印入口

  void processStop(void);                        ///< 执行停止操作
  void processPause(void);                       ///< 执行暂停操作
  void processResume(void);                      ///< 执行恢复操作

  UINT getTime(void);                             ///< 获取打印时间
private:
  void prepareStop(void);                        ///< 停止打印准备操作
  bool resumeBackToPrintPos(void);               ///< 恢复打印返回打印位置
  void pauseToResumeTemp(void);                  ///< 暂停恢复打印，温度恢复
  void pauseProcess(void);                       ///< 暂停打印执行
  void pauseToCoolDown(FLOAT coolDownFactor);    ///< 暂停打印超时，降低温度
  bool isResumeTempDone(void);                   ///< 执行恢复温度操作
  void processResumeFinish(void);                ///< 执行恢复操作完成
};

/**
 * 打印文件控制
 */
class PrintFileControl
{
public:
  PrintFileControl();
  bool open(void);                               ///< 打开文件
  void close(void);                              ///< 关闭文件
  void getGcodeBuf(void);                        ///< 获取gcode数组
  INT getPercent(void);                          ///< 获取当前打印进度
  void setSDMedium(void);                        ///< 设置当前文件存储介质
private:
  void getFileName(void);                        ///< 获取打印文件名
  void readBMPFile(CONST PCHAR fileName);        ///< 获取bmp文件
  void readFinish(void);                         ///< 文件读取结束
  bool readDataToBuf(void);                      ///< 读取文件数据到数组
};

extern PrintControl printControl;
extern PrintFileControl printFileControl;

#endif //PRINTCONTROL_H
