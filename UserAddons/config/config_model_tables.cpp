#include "config_model_tables.h"

#ifdef __cplusplus
extern "C" {
#endif

//機型尺寸表
const short model_id_table[MODEL_COUNT]  =
{
  //{x_max_pos,y_max_pos,z_max_pos}
  M14,       //M14
  M2030,     //M2030
  M2041,     //M2041
  M2048,     //M2048
  M3145,     //M3145
  M4141,     //M4141
  M4040,     //M4040
  M4141S,    //M4141S
  AMP410W,   //AMP410W
  M14R03,    //M14R03
  M2030HY,   //M2030HY
  M14S,      //M14S
  M3145S,    //M3145S
  M15,       //M15
  M3036,     //M3036
  M4141S_NEW,//M4141S_NEW M41S改版
  M41G,      //M41G
  M3145T,    //M3145T
  M3145K,    //M3145T
  K5         //K5
};

//機型尺寸表
const int model_size_table[MODEL_COUNT][3]  =
{
  //{x_max_pos,y_max_pos,z_max_pos}
  {148, 148, 158}, //M14
  {208, 208, 308}, //M2030
  {208, 208, 418}, //M2041
  {208, 208, 488}, //M2048
  {318, 318, 458}, //M3145
  {418, 418, 418}, //M4141
  {408, 408, 408}, //M4040
  {418, 418, 418}, //M4141S
  {418, 418, 418}, //AMP410W
  {148, 148, 158}, //M14R03
  {208, 208, 308}, //M2030HY
  {148, 148, 148}, //M14S
  {318, 318, 458}, //M3145S
  {153, 153, 168}, //M15
  {308, 308, 368}, //M3036
  {418, 418, 418}, //M4141S-NEW
  {418, 418, 432}, //M41G
  {318, 318, 458}, //M3145T
  {318, 318, 458}, //M3145K
  {208, 208, 300}  //K5
};

// 機型名稱表
const char *model_name_table[MODEL_COUNT] =
{
  //{x_max_pos,y_max_pos,z_max_pos}
  "M14",          //M14
  "M2030",        //M2030
  "M2041",        //M2041
  "M2048",        //M2048
  "M3145",        //M3145
  "K400/M4141",   //M4141
  "M4040TP",      //M4040
  "M4141S",       //M4141S
  "AMP410W",      //AMP410W
  "M14R03",       //M14R03
  "M2030HY",      //M2030HY
  "M14S",         //M14S
  "M3145S/S300",  //M3145S
  "M15",          //M15
  "M3036",        //M3036
  "M4141S_N",     //M4141S_NEW
  "S400/S1",      //M41G
  "M3145T",       //M3145T
  "K300/M3145K",  //M3145K
  "K5/K6"         //K5
};

#ifdef __cplusplus
} // extern "C" {
#endif



