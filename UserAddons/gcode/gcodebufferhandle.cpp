#include "gcodebufferhandle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threed_engine.h"
#include "globalvariables.h"
#include "sys_function.h"
#include "sysconfig_data.h"
#include "sg_util.h"
#include "Alter.h"

namespace gcode
{

#ifdef __cplusplus
  extern "C" {
#endif
#define ONECOLOR  ((char*)"M305")
#define TWOCOLOR  ((char*)"G428")
    // GcodeBufHandle variables
    static uint32_t ForceChksum_RetryCnt;                         /*!< 无$强制校验重试计数 */
    static uint32_t ForceChksum_ErrorCnt;                         /*!< 无$强制校验错误计数 */
    static uint32_t check_unknown_cmd_retry_count;                /*!< 未知指令校验重试计数 */
    static uint32_t check_unknown_cmd_error_count;                /*!< 未知指令校验错误计数 */
    static uint32_t checksum_error_counts;                        /*!< 已知指令校验重试计数 */
    static uint32_t checksum_retry_counts;                        /*!< 已知指令校验错误计数 */

    static volatile uint32_t cmd_buf_pos;                         /*!< 当前指令位置 */
    static volatile uint32_t leave_file_size;                     /*!< 剩余文件大小 */
    static volatile bool is_comment_mode;                                /*!< 是否为注释 */
    static volatile uint8_t command_buffer_count;               /*!< 指令数组计数 */

    void resetCmdBuf(void)
    {
      gcodeBufHandle.reset();
    }

    UINT8 GetGcodeFromBuf(void)
    {
      return gcodeBufHandle.getFromGcode();
    }


#ifdef __cplusplus
  } //extern "C" {
#endif

// ParseGcodeBufHandle variables
  static volatile char * volatile strchr_pointer;                                  /*!< just a pointer to find chars in the cmd string like X, Y, Z, E, etc */
  ParseGcodeBufHandle parseGcodeBufHandle;

// GcodeBufHandle variables
  GcodeBufHandle gcodeBufHandle;

  /**
   *
   */
  ParseGcodeBufHandle::ParseGcodeBufHandle()
  {
    strchr_pointer = NULL;
  }

  /**
   * [ParseGcodeBufHandle::codeValue description]
   * @return [description]
   */
  float ParseGcodeBufHandle::codeValue()
  {
    return strtof((char*)strchr_pointer + 1, NULL);
  }

  /**
   * [ParseGcodeBufHandle::codeValueLong description]
   * @return [description]
   */
  long ParseGcodeBufHandle::codeValueLong()
  {
    return (strtol((char*)strchr_pointer + 1, NULL, 10));
  }

  bool ParseGcodeBufHandle::codeSeen(const char code)
  {
    strchr_pointer = strchr((char*)command_buffer[command_buffer_tail], code);
    if(strchr_pointer != NULL)
      return true;
    else
      return false;
  }

  bool ParseGcodeBufHandle::codeSeenStr(const char * code)
  {
    char *strstr_pointer = strstr((char*)command_buffer[command_buffer_tail], code);
    if(strstr_pointer != NULL)
      return true;
    else
      return false;
  }

///////////////////////////////////////////////////////////////////////////////////////////

  /**
   *
   */
  GcodeBufHandle::GcodeBufHandle()
  {
    reset();
  }

  /**
   * [GcodeBufHandle::reset description]
   */
  void GcodeBufHandle::reset(void)
  {
    ForceChksum_RetryCnt = 0;
    ForceChksum_ErrorCnt = 0;
    check_unknown_cmd_retry_count = 0;
    check_unknown_cmd_error_count = 0;
    checksum_error_counts = 0;
    checksum_retry_counts = 0;
    cmd_buf_pos = 0;
    leave_file_size = 0;
    is_comment_mode = false;
    command_buffer_count = 0;
  }

  static void Judge_Color(void)
  {
    char * ForceVerifyPtr1 = strstr((char *)command_buffer[command_buffer_head],ONECOLOR);
    char * ForceVerifyPtr2 = strstr((char *)command_buffer[command_buffer_head],TWOCOLOR);
    if(ForceVerifyPtr1)
    {
      t_sys.enable_color_buf = 0;
    }
    else if(ForceVerifyPtr2)
    {
      t_sys.enable_color_buf = 1;
    }
    else
    {
      //测试固件需要判断是否设置好机型，否则异常
      t_sys.enable_color_buf = ((1==t_sys_data_current.enable_color_mixing) && (1 == t_sys_data_current.have_set_machine));
    }
  }

// 获取指令字符
  static __inline void getCmdChar(const char value)
  {
    if ((value == ';') || (value == '('))
    {
      is_comment_mode = true;  //检测到分号
    }
    if (!is_comment_mode)   //存放数据，过滤掉分号后的数据
    {
      if (command_buffer_count >= CMD_BUF_SIZE)
      {
        command_buffer[command_buffer_head][CMD_BUF_SIZE - 1] = '\r';  //超过缓冲区，截断数据，以防溢出
      }
      else
      {
        command_buffer[command_buffer_head][command_buffer_count] = value;
        ++command_buffer_count;
      }
    }
  }

// 修复联机打印第一层没变化，cura生成的gcode多了空格，导致校准异常跳出该指令
// 软件修复后，可删除该逻辑
  static __inline void fixCuraErr(void)
  {
    char *pStr = strstr((char*)command_buffer[command_buffer_head]," E ");
    if(NULL != pStr)
    {
      strcpy(pStr+2, pStr+3);
      command_buffer_count--;
    }
  }

// 发送指令
  static __inline void sendCmd(void)
  {
    char str[20];
    serial_print[0] = false;
    serial_print[1] = false;
    memset(str, 0, sizeof(char)*20);
    (void)snprintf(&str[0], 20, " P%u", t_gui.file_position); // 加上文件位置
    for(int i = 0; i < 20; i++)
    {
      if(command_buffer_count + i < CMD_BUF_SIZE)
      {
        command_buffer[command_buffer_head][command_buffer_count+i] = str[i];
      }
    }
    sys_send_gcode_cmd_delay(false);
    command_buffer_count = 0; //clear buffer
    is_comment_mode = false;
  }

  bool GcodeBufHandle::getFromGcode(void)
  {
    char sd_char = file_read_buf[file_read_buf_index];
    ++file_read_buf_index;

    if ((sd_char == '\r')  || (sd_char == '\n'))
    {
      command_buffer[command_buffer_head][command_buffer_count] = 0; // 添加字符串结束符

      fixCuraErr();

      if (command_buffer_count == 0)                // 空行或直接分号的注释行，跳过
      {
        is_comment_mode = false;
        return 0;
      }
      else if (is_comment_mode)       //有带分号注释的Gcode命令行
      {
        Judge_Color();  //支持混色和单色打印
        sendCmd();
        return 1;
      }
      else //不带分号注释的Gcode命令行
      {
        //当前命令行校验
        if(verifyCmdBuf() == false)
        {
          for(int i = 0; i < CMD_BUF_SIZE; i++)
          {
            command_buffer[command_buffer_head][i] = 0;
          }
          command_buffer_count = 0;
          is_comment_mode = false;
          return 0;
        }
        sendCmd();
        cmd_buf_pos = file_read_buf_index;
        leave_file_size = t_gui.file_size;
        return 1;
      }
    }
    else
    {
      getCmdChar(sd_char);
    }
    return 0;
  }

  void GcodeBufHandle::updateCmdBuf(void)
  {
    if(!t_sys.enable_color_buf) //非混色，跳过更新指令数组
    {
      return;
    }
    char decryptBuf_tmp[CMD_BUF_SIZE];
    memset(decryptBuf_tmp, 0, sizeof(char)*CMD_BUF_SIZE);
    int pos = 0;
    while (pos < command_buffer_count)
    {
      // 扫描到校验字符$，退出
      if (((pos + 1) < CMD_BUF_SIZE) && (command_buffer[command_buffer_head][pos] == ' ') && (command_buffer[command_buffer_head][pos + 1] == '$'))
        break;
      // 解码当前字符
      decryptBuf_tmp[pos] = sg_util::get_decryption_code(command_buffer[command_buffer_head][pos], 20, pos);
      // 计数超出最大值，退出
      ++pos;
      if (pos == command_buffer_count)
        break;
    }

    command_buffer_count = pos;
    // 将cmdbuffer_temp复制到curr_buf
    for(int i = 0; i < CMD_BUF_SIZE; i++)
    {
      if(i < command_buffer_count)
      {
        command_buffer[command_buffer_head][i] = decryptBuf_tmp[i];
      }
      else
      {
        command_buffer[command_buffer_head][i] = 0;
      }
    }
    command_buffer[command_buffer_head][command_buffer_count] = 0;  // 添加字符串结束符
  }

//仅command_buffer校验用
  int GcodeBufHandle::getCheckSum(void)
  {
    // 初始化校验值
    int chksum = command_buffer[command_buffer_head][0];
    for(int i = 1; i< CMD_BUF_SIZE; ++i)
    {
      // 扫描到校验字符$，退出
      if(((i + 1) < CMD_BUF_SIZE) && (command_buffer[command_buffer_head][i] == ' ') &&( command_buffer[command_buffer_head][i+1] == '$'))
        break;
      chksum ^= command_buffer[command_buffer_head][i];
    }
    return chksum;
  }

  void GcodeBufHandle::verifyCmdBufCount(int loopCnt, int &currCnt, uint32_t &retryCnt, uint32_t &ErrorCnt)
  {
    // 校验次数小于loopCnt，重新读取指令
    if(currCnt < loopCnt)
    {
      file_read_buf_index = cmd_buf_pos;       // 设置pos
      if(leave_file_size)//20170830防止file_size被改为0
        t_gui.file_size = leave_file_size;
      ++retryCnt;                        // 重读计数
      ++currCnt;                         // 校验次数加一
    }
    // 校验次数等于loopCnt，指令错误，跳过该指令
    else if(currCnt == loopCnt)
    {
      ++ErrorCnt;                        // 错误计数
      currCnt = 0;                       // 校验次数重置为0
    }
    command_buffer_count = 0;            // 清空行字符计数
  }

  bool GcodeBufHandle::verifyKnownCmd(bool isChkSumCorrect)
  {
    //known command
    static int unknown_cmd_counts=0;
    static int chksum_loop_cnt = 0;
    char first_code = command_buffer[command_buffer_head][0];
    bool isKnownCmd = ((first_code == 'G') || (first_code == 'M') || (first_code == 'T')); // 是否为已知指令
    // 已知指令
    if(isKnownCmd)
    {
      if (!isChkSumCorrect)
      {
        verifyCmdBufCount(20, chksum_loop_cnt, checksum_retry_counts, checksum_error_counts);
        return false;//return;
      }
      else
      {
        chksum_loop_cnt = 0;
      }
      unknown_cmd_counts=0;
    }
    else
    {
      //unknown command
      verifyCmdBufCount(3, unknown_cmd_counts, check_unknown_cmd_retry_count, check_unknown_cmd_error_count);
      return false;
    }
    return true;
  }

  bool GcodeBufHandle::verifyCmdBuf(void)
  {
    // 定义强制校验次数
    static int verifyForceCount = 0;
    // 获取美元符号位置
    char* find_dollar_char = strchr((char*)command_buffer[command_buffer_head], '$');
    // M305 S1时，开启强制模式，指令没有校验符号$，直接跳过该指令
    if ((find_dollar_char == NULL) && (0U != t_gui_p.m305_is_force_verify))
    {
      verifyCmdBufCount(3, verifyForceCount, ForceChksum_RetryCnt, ForceChksum_ErrorCnt);
      return false;
    }
    else
    {
      verifyForceCount = 0; // 校验次数重置
    }

    if (find_dollar_char != NULL)
    {
      int get_buf_chksum = strtod( find_dollar_char + 1, NULL);// 获取命令行附加校验值
      int cal_buf_chksum = getCheckSum(); // 计算命令行校验值
      updateCmdBuf(); // 解码到当前buffer
      bool isChkSumCorrect = ((get_buf_chksum == cal_buf_chksum) || ((cal_buf_chksum^' ') == get_buf_chksum))?true:false;               // 比较并计算校验结果
      return verifyKnownCmd(isChkSumCorrect);
    }
    return true;
  }


}



