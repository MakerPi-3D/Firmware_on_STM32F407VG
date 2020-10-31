#ifndef FILAMENTCONTROL_H
#define FILAMENTCONTROL_H
#include "wbtypes.h"
#ifdef __cplusplus
extern "C" {
#endif
  void block_detect_change_status(void);
  void blockDetect_process();
  void BlockDetectControl(void);
#ifdef __cplusplus
}
#endif

/**
 * 进退丝操作
 */
class FilamentControl
{
public:
  FilamentControl();
  void startLoad(void);                ///< 开始进丝
  void startUnload(void);              ///< 开始退丝
  void process(void);                  ///< 执行进退丝入口
  void cancelProcess(void);            ///< 取消进退丝
  void resetStatus(void);              ///< 重置进退丝状态
private:
  void prepare(void);                  ///< 准备进退丝操作
  void processLoad(void);              ///< 执行进丝操作
  void processUnload(void);            ///< 执行退丝操作
  void exit(bool isCancel);            ///< 退出进退丝
private:
  UINT8 startLoadFlag;               /*!< 开始进丝标志位 */
  UINT8 startUnloadFlag;             /*!< 开始退丝标志位 */
  UINT8 timeOutFlag;                 /*!< 进退丝超时标志位 */
  ULONG timeOutTickCount;      /*!< 进退丝超时计数 */
};

class BlockDetect
{
public:
  BlockDetect();
  void init();
  void ttc_update();
  void clean();
  void available();

  UINT32 encoderPosition;
  UINT32 encoderPosition_save;
  UINT8 encoder_update_lock;   //编码更新锁定，避免误操作数据

private:
  UINT8 lastEncoderBits;       //
  volatile UINT8 buttons;      //the last checked buttons in a bit array.
  INT encoderDiff;               /* encoderDiff is updated from interrupt context and added to encoderPosition every LCD update */
  volatile INT encoderDiffSub;   //反向编码递增器
  UINT8 ttc_update_lock;

};

extern FilamentControl filamentControl;
extern BlockDetect blockDetect;

#endif // FILAMENTCONTROL_H
