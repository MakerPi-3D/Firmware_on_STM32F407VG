#ifndef MK_UTIL_H
#define MK_UTIL_H

namespace sg_util
{

#ifdef __cplusplus
extern "C" {
#endif

  extern bool is_float_data_equivalent(const float p_a, const float p_b);
  extern char get_decryption_code (char source, int key, int pos);

#ifdef __cplusplus
} /* extern "C" { */
#endif
extern "C++" template<typename T> inline void assert_param_p(T v) {}

extern "C++" template<typename T> inline T square_p(const T x)
{
  const T result = x*x;
  return (result);
}

extern "C++" template<typename T> inline T min_p(const T a, const T b)
{
  T result = b;
  if(a < b)
  {
    result = a;
  }
  return (result);
}

extern "C++" template<typename T> inline T max_p(const T a, const T b)
{
  T result = b;
  if(a > b)
  {
    result = a;
  }
  return (result);
}

extern "C++" template<typename T> inline void noless_p(T &v, const T n)
{
  if (v < n)
  {
    v = n;
  }
}

extern "C++" template<typename T> inline void nomore_p(T &v, const T n)
{
  if (v > n)
  {
    v = n;
  }
}

extern "C++" template<typename T> inline T constrain_p(T amt, T low, T high)
{
  T result = amt;
  if(amt < low)
  {
    result = low;
  }
  else
  {
    if(amt > high)
    {
      result = high;
    }
  }
  return (result);
}

}

#endif


