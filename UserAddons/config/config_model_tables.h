#ifndef CONFIG_MODEL_TABLES_H
#define CONFIG_MODEL_TABLES_H

#ifdef __cplusplus
extern "C" {
#endif

  // 機型ID
  enum ModelId
  {
    M14 = 0,
    M2030 = 1,
    M2041 = 2,
    M2048 = 3,
    M3145 = 4,
    M4141 = 5,
    M4040 = 6, // 日本专用
    M4141S = 7,
    AMP410W = 8,
    M14R03 = 9,
    M2030HY = 10,
    M14S = 11,
    M3145S = 12,
    M15 = 13,
    M3036 = 14,
    M4141S_NEW = 15, //M4141S_NEW M41S改版
    M41G = 16,
    M3145T = 17,
    M3145K = 18,
    K5 = 19,
    MODEL_COUNT = 20 // 机型数量
  };

  // 機型表
  extern const short model_id_table[MODEL_COUNT];
  extern const int   model_size_table[MODEL_COUNT][3];
  extern const char* model_name_table[MODEL_COUNT];


#ifdef __cplusplus
} // extern "C" {
#endif


#endif // CONFIG_MODEL_TABLES_H

