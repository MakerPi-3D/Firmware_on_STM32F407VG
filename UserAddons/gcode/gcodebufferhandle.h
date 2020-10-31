#ifndef GCODEBUFFERHANDLE_H
#define GCODEBUFFERHANDLE_H
#include <string.h>
#include <stdint.h>
namespace gcode
{

#ifdef __cplusplus
  extern "C" {
#endif

    extern uint8_t GetGcodeFromBuf(void);
    extern void resetCmdBuf(void);

#ifdef __cplusplus
  } // extern "C" {
#endif


  /**
   * 解析gcode指令
   */
  class ParseGcodeBufHandle
  {
  public:
    ParseGcodeBufHandle();
    bool codeSeen(const char code);                   ///< 查找字符并定位该字符位置
    bool codeSeenStr(const char* code);               ///< 查找字符串并定位该字符串位置
    float codeValue();                                ///< 获取查找字符或字符串后面所带的数值，整型
    long codeValueLong();                             ///< 获取查找字符或字符串后面所带的数值，浮点型
  };

  /**
   * Gcode指令获取
   */
  class GcodeBufHandle
  {
  public:
    GcodeBufHandle();
    bool getFromGcode(void);      ///< 获取指令
    void reset(void);                                                                          ///< 重置指令
  private:
    int getCheckSum(void);                                                      ///< 获取校验码
    void verifyCmdBufCount(int loopCnt, int &currCnt, uint32_t &retryCnt, uint32_t &ErrorCnt); ///< 校验计数
    bool verifyCmdBuf(void);                     ///< 校验指令
    void updateCmdBuf(void);                                         ///< 更新指令
    bool verifyKnownCmd(bool isChkSumCorrect);                                ///< 校验已知指令
  };

  extern ParseGcodeBufHandle parseGcodeBufHandle;
  extern GcodeBufHandle gcodeBufHandle;


}

#endif // GCODEBUFFERHANDLE_H




