#include "print_task.h"
#include "config_model_tables.h"
#include "gcode.h"
#include "user_interface.h"
#include "globalvariables.h"
#include <string.h>
#include "Alter.h"
#ifdef __cplusplus
extern "C" {
#endif
  extern void buflen_pro(void);

  void print_task_loop(void)
  {
    if (sys_receive_gcode_cmd())
    {
      process_commands();  //处理gcode命令
      if(serial_print[0])
      {
        buflen_pro();
      }
    }
		(void)sys_os_delay(10); //延时以让低优先级的任务执行
  }

#ifdef __cplusplus
} //extern "C"
#endif





