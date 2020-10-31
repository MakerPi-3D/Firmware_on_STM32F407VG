#include "machine_custom.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "config_model_tables.h"
#include <string.h>
#include "ConfigurationStore.h"
#include "machinecustom.h"
#include  "interface.h"
#include "globalvariables.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void temperature_set_heater_maxtemp(INT axis, INT value);

// 定制机型初始化
// 不要删除已有定制型号，避免更新固件后，定制机型查找不到导致配置混乱
void machine_custom_init(void)
{
  if ((M3145T == t_sys_data_current.model_id) && (0 != t_sys_data_current.custom_model_id))
  {
    t_sys_data_current.custom_model_id = 0;
    SaveCustomModelId();
  }

  //  // 20181113：M41G使用热电偶，其他机器使用热敏电阻
  //  //     热敏电阻极限温度为300度，使用查表方式匹配温度
  //  //     热电偶温度对应关系3.3v对应660度
  //  //     目前热敏电阻查表280以后的数据为伪数据，实际300度对应为320度
  //  if(1 == t_sys_data_current.enable_v5_extruder || 2 == t_sys_data_current.enable_v5_extruder)
  //  {
  //    temperature_set_heater_maxtemp(0, 325); // 可调温度范围0到280
  //  }

  if (0 == t_sys_data_current.custom_model_id) // 常规
  {
    if (t_sys_data_current.enable_color_mixing)
    {
      // K5 混色 挤出器直径为7.4mm
      if (K5 == t_sys_data_current.model_id)
      {
        if (t_sys_data_current.ui_number == 2)
          (void)strcpy(t_sys.model_str, "X5/X6/K5 MIX");
        else
          (void)strcpy(t_sys.model_str, "X5/X6");
      }
      else
      {
        strcat(t_sys.model_str, "X");
      }
    }
    else
    {
      if (K5 == t_sys_data_current.model_id)
      {
        if (t_sys_data_current.ui_number == 2)
        {
          strcat(t_sys.model_str, "/K5 PLUS");
        }
      }
    }

    if (K5 == t_sys_data_current.model_id)
    {
      if (t_sys_data_current.enable_color_mixing && !t_sys_data_current.IsMechanismLevel)
      {
        t_sys_data_current.enable_bed_level = 0;
      }
    }

    // 根据文档"Document/机型定位对比表.xlsx"修改
    if (1 == t_sys_data_current.enable_high_temp)
    {
      if ((K5 == t_sys_data_current.model_id) || (M3145K == t_sys_data_current.model_id) || (M3145S == t_sys_data_current.model_id))
      {
        temperature_set_heater_maxtemp(0, 305);
      }
      else if (M4141 == t_sys_data_current.model_id)
      {
        temperature_set_heater_maxtemp(0, 275);
      }
      else if (M41G == t_sys_data_current.model_id)
      {
        temperature_set_heater_maxtemp(0, 525);
      }
    }
    else if (0 == t_sys_data_current.enable_high_temp)
    {
      temperature_set_heater_maxtemp(0, 275);
    }
  }
  else if (1 == t_sys_data_current.custom_model_id) // 宋沙沙广州尚天客户定制机型 M2030D 改为 HW210-B，M14R03 改为 HW150A
  {
    if ((M2030 == t_sys_data_current.model_id) && t_sys_data_current.enable_color_mixing)
    {
      (void)strcpy(t_sys.model_str, "HW210-B");
    }
    else if (M14R03 == t_sys_data_current.model_id)
    {
      (void)strcpy(t_sys.model_str, "HW150A");
    }
  }
  else if (2 == t_sys_data_current.custom_model_id) // 宋沙沙南京銮清电子 M2030 改为 TOP—ONE
  {
    if (M2030 == t_sys_data_current.model_id)
    {
      (void)strcpy(t_sys.model_str, "TOP-ONE");
    }
  }
  else if (3 == t_sys_data_current.custom_model_id) // 銮清定制机型“3D Printer”
  {
    //M2030X显示"3D Printer" ；M2048X其实是銮清M2045X,客户要求显示"3D Printer"201798
    if (((M2030 == t_sys_data_current.model_id) || (M2048 == t_sys_data_current.model_id)) && t_sys_data_current.enable_color_mixing)
    {
      (void)strcpy(t_sys.model_str, "3D Printer");
    }
  }
  else if (4 == t_sys_data_current.custom_model_id) // 东莞驰一三维科技有限公司 C1414（M14），C2030（M2030），C3144（M3145），C4141（M4141），C2030X(M2030)
  {
    if ((M14 == t_sys_data_current.model_id) || (M14R03 == t_sys_data_current.model_id))
    {
      (void)strcpy(t_sys.model_str, "C1414");
    }
    else if (M3145 == t_sys_data_current.model_id)
    {
      (void)strcpy(t_sys.model_str, "C3144");
    }
    else if (M4141 == t_sys_data_current.model_id)
    {
      (void)strcpy(t_sys.model_str, "C4141");
    }
    else if (M2030 == t_sys_data_current.model_id)
    {
      (void)strcpy(t_sys.model_str, "C2030");

      if (t_sys_data_current.enable_color_mixing)
      {
        (void)strcpy(t_sys.model_str, "C2030X");
      }
    }
  }
  // 日本，M3145白色机器定制
  // 1.送料和退料的默认温度请改成160度，手动调节可以到230度
  // 2.类似以前的低温固件一样，打印温度150度以上马达即可转动
  // 3.取消ABS予熱和PLA予熱两个按钮，增加ベッド予熱（意为热床预热），按ベッド予熱的时候，喷嘴不预热，热床预热到100度。
  // 喷嘴为快拆
  else if (5 == t_sys_data_current.custom_model_id)
  {
    sg_grbl::filament_load_unload_temp = 160;
    motion_3d.extrude_min_temp = 150;
    t_sys_data_current.enable_soft_filament = 1; // 打印软料，开启软料功能

    if (M3145 == t_sys_data_current.model_id)
    {
      (void)strcpy(t_sys.model_str, "M3145TP MEDICAL MODEL");
    }

    t_sys.is_detect_extruder_thermistor = 1;
  }
  else if (6 == t_sys_data_current.custom_model_id)
  {
    //    (void)strcpy(t_sys.model_str, modelStr);
    if (0 == t_sys_data_current.enable_cavity_temp)
    {
      t_sys_data_current.enable_cavity_temp = 1;
    }

    if (t_sys_data_current.enable_color_mixing)
    {
      strcat(t_sys.model_str, "X");
    }
  }
  else if (7 == t_sys_data_current.custom_model_id)
  {
    // 日本，高温M2030机器
    sg_grbl::filament_load_unload_temp = 310; // 进丝温度
    temperature_set_heater_maxtemp(0, 325); // 喷嘴最大温度，可打印PC

    //    t_sys_data_current.enable_v5_extruder = 1;
    //    (void)strcpy(t_sys.model_str, modelStr);
    if (t_sys_data_current.enable_color_mixing)
    {
      strcat(t_sys.model_str, "X");
    }

    (void)strcat(t_sys.model_str, "-HI TEMP");
  }
  else if (8 == t_sys_data_current.custom_model_id)
  {
    // M3145S开启腔体温度功能、自动调平功能
    // M3145K开启自动调平
    // K5开启自动调平
    if (M3145K == t_sys_data_current.model_id)
    {
      t_sys_data_current.enable_bed_level = 2;
      t_sys.is_bed_level_down_to_zero = 1;
    }
    else if (M3145S == t_sys_data_current.model_id)
    {
      if ((0 == t_sys_data_current.enable_cavity_temp) || (2 != t_sys_data_current.enable_bed_level))
      {
        t_sys_data_current.enable_cavity_temp = 1;
        t_sys_data_current.enable_bed_level = 2;
      }

      t_sys.is_bed_level_down_to_zero = 1;
    }
  }
  else if (9 == t_sys_data_current.custom_model_id)
  {
    t_sys_data_current.enable_type_of_thermistor = 1;
    temperature_set_heater_maxtemp(0, 525); // 喷嘴最大温度，可打印PC

    if ((0 == t_sys_data_current.enable_cavity_temp) || (1 != t_sys_data_current.enable_v5_extruder))
    {
      t_sys_data_current.enable_cavity_temp = 1;
      t_sys_data_current.enable_v5_extruder = 1;
    }

    //    (void)strcpy(t_sys.model_str, modelStr);
    if (t_sys_data_current.enable_color_mixing)
    {
      strcat(t_sys.model_str, "X");
    }
  }
  else if (10 == t_sys_data_current.custom_model_id)
  {
    if (M3145K == t_sys_data_current.model_id)
    {
      if (2 == t_sys_data_current.enable_bed_level)
      {
        t_sys.is_bed_level_down_to_zero = 1;
      }
    }

    if (t_sys_data_current.enable_color_mixing)
    {
      strcat(t_sys.model_str, "X");
    }
  }
  else if (11 == t_sys_data_current.custom_model_id)
  {
    sg_grbl::filament_load_unload_temp = 180;
    motion_3d.extrude_min_temp = 170;
    t_sys.is_granulator = 1;
  }
  else if (12 == t_sys_data_current.custom_model_id)
  {
    t_sys_data_current.enable_type_of_thermistor = 1;
    sg_grbl::filament_load_unload_temp = 250; // 进丝温度

    if (M3145S == t_sys_data_current.model_id)
    {
      temperature_set_heater_maxtemp(0, 325); // 喷嘴最大温度，可打印PC
    }
    else if (K5 == t_sys_data_current.model_id)
    {
      temperature_set_heater_maxtemp(0, 325); // 喷嘴最大温度，可打印PC
    }
    else if (M41G == t_sys_data_current.model_id)
    {
      temperature_set_heater_maxtemp(0, 425); // 喷嘴最大温度，可打印PC
    }

    if (t_sys_data_current.enable_color_mixing)
    {
      strcat(t_sys.model_str, "X");
    }
  }
  else if (13 == t_sys_data_current.custom_model_id)
  {
    // 万物打印、M4141S、M4141S_NEW 2017525前的机器
    if ((M4141S == t_sys_data_current.model_id) || (M4141S_NEW == t_sys_data_current.model_id) || (AMP410W == t_sys_data_current.model_id))
    {
      motion_3d_model.enable_invert_dir[X_AXIS] = false;
      motion_3d_model.enable_invert_dir[Y_AXIS] = true;
      motion_3d_model.enable_invert_dir[Z_AXIS] = false;
      strcat(t_sys.version_str, "_O");
    }
  }
  else if (15 == t_sys_data_current.custom_model_id)
  {
    if (M3145S == t_sys_data_current.model_id)
    {
      temperature_set_heater_maxtemp(0, 275);
      t_gui_p.cavity_temp_max_value = 80;
    }
  }

  if (t_sys_data_current.is_2GT)
  {
    strcat(t_sys.model_str, " 2GT");
  }

  if (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id)
  {
    motion_3d.extrude_min_temp = 150;   // 日本可以打印低温耗材

    if (t_sys_data_current.enable_v5_extruder)
    {
      temperature_set_heater_maxtemp(0, 325); // 喷嘴最大温度，可打印PC
    }
  }

  // 自动调平类型为2，启动归零向下开关
  if (2 == t_sys_data_current.enable_bed_level)
  {
    //    if(1U == t_sys_data_current.IsMechanismLevel)
    t_sys.is_bed_level_down_to_zero = 1;
    //    else
    //      t_sys.is_bed_level_down_to_zero = 0;
  }

  if (1 == t_sys_data_current.enable_type_of_thermistor)
  {
    strcat(t_sys.model_str, "_TC1");
  }

  if (1 == t_sys_data_current.enable_high_temp)
  {
    strcat(t_sys.model_str, "_HT");
  }

  //  USER_DbgLog("t_sys.enable_soft_filament=%d",t_sys_data_current.enable_soft_filament);
  //  // 罗建旭需要60-300度的喷嘴温度，型号可以M2030或者M3145，用于测试低温材料。
  //  motion_3d.extrude_min_temp = 40;
  ////  filament_load_unload_temp = 310; // 进丝温度
  //  temperature_set_heater_maxtemp(0, 350); // 喷嘴最大温度，可打印PC
  //  (void)strcat(t_sys.model_str, "-TEST");
}

#ifdef __cplusplus
} //extern "C" {
#endif

