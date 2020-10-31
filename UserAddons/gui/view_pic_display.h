#ifndef VIEW_PIC_DISPLAY_H
#define VIEW_PIC_DISPLAY_H

#include "wbtypes.h"
#include "view_common.h"

#ifdef __cplusplus
extern "C" {
#endif



  void display_picture(INT PictureName);
  void DisplayLogoPicture(INT PictureName);


//画进度条
#define BarWidth 382  //进度条长度
#define BarHeight 21  //进度条宽度
#define X_BEGIN 75    //进度条x轴起始位置
#define Y_BEGIN 289   //进度条y轴起始位置
  UINT32 Draw_progressBar_new(UINT32 Printfilesize,UINT32 Filesize, INT x, INT y, INT x_max, INT y_max);
  UINT32 Draw_progressBar(UINT32 Printfilesize,UINT32 Filesize);
  void diplayBMP(UINT y_offset);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // VIEW_PIC_DISPLAY_H
