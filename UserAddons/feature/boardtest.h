#ifndef TESTFIXTURE_H
#define TESTFIXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wbtypes.h"
//#include "stm32f4xx_hal.h"

#ifdef __cplusplus
} //extern "C" {
#endif

class BoardTest
{
public:
  BoardTest();
  bool guiInterface(void);
  void displayInit(void);
  void displayText(void);
  bool touchCheck(void);
  void modelSelect(void);

  void toggleStepStatus(void);
  void toggleFanStatus(void);
  void toggleHeatStatus(void);

  void process(void);

  void runMaxPos(void);
  bool calHeatTimeGui(void);
  void calHeatTime(void);

  UINT8 ERRTouchCount(void);
  void PressureTest(void);

private:
  UINT8 isStepTest;
  UINT8 isFanTest;
  UINT8 isHeatTest;

  CHAR nozzleHeatTimeStr[32];
  CHAR bed50HeatTimeStr[32];
  CHAR bed70HeatTimeStr[32];
  CHAR bed115HeatTimeStr[32];
  INT heatStatus;
  UINT32 clockTime;

  UINT8 processOn;

  void stepTest(void);
  void fanTest(void);
  void heatTest(void);

  bool calHeatTimeGuiTouchCheck(void);
};

extern BoardTest boardTest;

#endif // TESTFIXTURE_H

