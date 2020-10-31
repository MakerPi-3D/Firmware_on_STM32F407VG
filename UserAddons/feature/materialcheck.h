#ifndef MATERIALCHECK_H
#define MATERIALCHECK_H


/**
 * 断料检测
 */
class MaterialCheck
{
public:
  MaterialCheck();
  void init(void);                    ///< 断料检测初始化
  void process(void);                 ///< 断料检测入口
};

extern MaterialCheck materialCheck;
#endif // MATERIALCHECK_H
