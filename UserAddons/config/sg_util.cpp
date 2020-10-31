#include "sg_util.h"

namespace sg_util
{

#ifdef __cplusplus
extern "C" {
#endif
  /* 判断两个浮点数是否相等 */
  bool is_float_data_equivalent(const float p_a, const float p_b)
  {
    const float EPSINON = 0.000001F;
    bool result = false;
    const float x = p_a - p_b; // float
    if((x >= -EPSINON)&& (x <= EPSINON))
    {
      result = true;
    }
    return (result);
  }

  // 混色简单加密
  char get_decryption_code (char source, int key, int pos)
  {
    if ((source >= 'a') && (source <= 'z'))
    {
      source = source - ((key + pos) % 26);
      if (source < 'a')
      {
        source += 26;
      }
    }
    else if ((source >= 'A') && (source <= 'Z'))
    {
      source = source - ((key + pos) % 26);
      if (source < 'A')
      {
        source += 26;
      }
    }
    else if ((source >= '0') && (source <= '9'))
    {
      source = source - ((key + pos) % 10);
      if (source < '0')
      {
        source += 10;
      }
    }
    return source;
  }
#ifdef __cplusplus
} /* extern "C" { */
#endif

}












