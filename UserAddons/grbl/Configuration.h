#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#pragma once
#include <stdbool.h>

// This configuration file contains the basic settings.
// Advanced settings can be found in Configuration_adv.h
// BASIC SETTINGS: select your board type, temperature sensor type, axis scaling, and endstop configuration

//===========================================================================
//============================= DELTA Printer ===============================
//===========================================================================
// For a Delta printer replace the configuration files with the files in the
// example_configurations/delta directory.
//

//===========================================================================
//============================= SCARA Printer ===============================
//===========================================================================
// For a Delta printer replace the configuration files with the files in the
// example_configurations/SCARA directory.
//


// Define this to set a unique identifier for this printer, (Used by some programs to differentiate between machines)
// You can use an online service to generate a random UUID. (eg http://www.uuidgenerator.net/version4)
// #define MACHINE_UUID "00000000-0000-0000-0000-000000000000"

// This defines the number of extruders
#define EXTRUDERS 1

//===========================================================================
//=============================Thermal Settings  ============================
//===========================================================================
//
//--NORMAL IS 4.7kohm PULLUP!-- 1kohm pullup can be used on hotend sensor, using correct resistor and table
//
//// Temperature sensor settings:
// -2 is thermocouple with MAX6675 (only for sensor 0)
// -1 is thermocouple with AD595
// 0 is not used
// 1 is 100k thermistor - best choice for EPCOS 100k (4.7k pullup)
// 2 is 200k thermistor - ATC Semitec 204GT-2 (4.7k pullup)
// 3 is Mendel-parts thermistor (4.7k pullup)
// 4 is 10k thermistor !! do not use it for a hotend. It gives bad resolution at high temp. !!
// 5 is 100K thermistor - ATC Semitec 104GT-2 (Used in ParCan & J-Head) (4.7k pullup)
// 6 is 100k EPCOS - Not as accurate as table 1 (created using a fluke thermocouple) (4.7k pullup)
// 7 is 100k Honeywell thermistor 135-104LAG-J01 (4.7k pullup)
// 71 is 100k Honeywell thermistor 135-104LAF-J01 (4.7k pullup)
// 8 is 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup)
// 9 is 100k GE Sensing AL03006-58.2K-97-G1 (4.7k pullup)
// 10 is 100k RS thermistor 198-961 (4.7k pullup)
// 11 is 100k beta 3950 1% thermistor (4.7k pullup)
// 12 is 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup) (calibrated for Makibox hot bed)
// 13 is 100k Hisens 3950  1% up to 300°C for hotend "Simple ONE " & "Hotend "All In ONE"
// 20 is the PT100 circuit found in the Ultimainboard V2.x
// 60 is 100k Maker's Tool Works Kapton Bed Thermistor beta=3950
//
//    1k ohm pullup tables - This is not normal, you would have to have changed out your 4.7k for 1k
//                          (but gives greater accuracy and more stable PID)
// 51 is 100k thermistor - EPCOS (1k pullup)
// 52 is 200k thermistor - ATC Semitec 204GT-2 (1k pullup)
// 55 is 100k thermistor - ATC Semitec 104GT-2 (Used in ParCan & J-Head) (1k pullup)
//
// 1047 is Pt1000 with 4k7 pullup
// 1010 is Pt1000 with 1k pullup (non standard)
// 147 is Pt100 with 4k7 pullup
// 110 is Pt100 with 1k pullup (non standard)

#define TEMP_SENSOR_0 1//20
#define TEMP_SENSOR_1 0
#define TEMP_SENSOR_2 0
#define TEMP_SENSOR_BED 1

// The minimal temperature defines the temperature below which the heater will not be enabled It is used
// to check that the wiring to the thermistor is not broken.
// Otherwise this would lead to the heater being powered on all the time.
#define HEATER_0_MINTEMP 1
#define HEATER_1_MINTEMP 1
#define HEATER_2_MINTEMP 1
#define BED_MINTEMP 1

// When temperature exceeds max temp, your heater will be switched off.
// This feature exists to protect your hotend from overheating accidentally, but *NOT* from thermistor short/failure!
// You should use MINTEMP for thermistor short/failure protection.
//#define TEST_300TEMP
#ifdef TEST_300TEMP
#define HEATER_0_MAXTEMP 350
#else
#define HEATER_0_MAXTEMP 275
#endif
#define HEATER_1_MAXTEMP 275
#define HEATER_2_MAXTEMP 275
#define BED_MAXTEMP 150

// PID settings:
// Comment the following line to disable PID and enable bang-bang.
#define PIDTEMP
#define BANG_MAX 255.0f // limits current to nozzle while in bang-bang mode; 255=full current
#define PID_MAX BANG_MAX // limits current to nozzle while PID is active (see PID_FUNCTIONAL_RANGE below); 255=full current
#ifdef PIDTEMP
#define PID_FUNCTIONAL_RANGE 20 // If the temperature difference between the target temperature and the actual temperature
// is more then PID_FUNCTIONAL_RANGE then the PID will be shut off and the heater will be set to min/max.
#define PID_INTEGRAL_DRIVE_MAX PID_MAX  //limit for the integral term
#define K1 0.95f //smoothing factor within the PID
#define PID_dT ((16.0f * 8.0f)/((F_CPU / 64.0f) / 256.0f)) //sampling period of the temperature routine

// If you are using a pre-configured hotend then you can use one of the value sets by uncommenting it
// Ultimaker
#define  DEFAULT_Kp 10.0f
#define  DEFAULT_Ki 0.5f
#define  DEFAULT_Kd 50.0f
#endif // PIDTEMP

// This sets the max power delivered to the bed, and replaces the HEATER_BED_DUTY_CYCLE_DIVIDER option.
// all forms of bed control obey this (PID, bang-bang, bang-bang with hysteresis)
// setting this to anything other than 255 enables a form of PWM to the bed just like HEATER_BED_DUTY_CYCLE_DIVIDER did,
// so you shouldn't use it unless you are OK with PWM on your bed.  (see the comment on enabling PIDTEMPBED)
#define MAX_BED_POWER 255 // limits duty cycle to bed; 255=full current



//this prevents dangerous Extruder moves, i.e. if the temperature is under the limit
//can be software-disabled for whatever purposes by
#define PREVENT_DANGEROUS_EXTRUDE
//if PREVENT_DANGEROUS_EXTRUDE is on, you can still disable (uncomment) very long bits of extrusion separately.
#define PREVENT_LENGTHY_EXTRUDE

#define EXTRUDE_MINTEMP  170
#define EXTRUDE_MAXLENGTH 0 //prevent extrusion of very large distances.

//===========================================================================
//=============================Mechanical Settings===========================
//===========================================================================

// Uncomment the following line to enable CoreXY kinematics
// #define COREXY

#define INVERT_X_DIR true     // for Mendel set to false, for Orca set to true
#define INVERT_Y_DIR false    // for Mendel set to true, for Orca set to false
#define INVERT_Z_DIR true     // for Mendel set to false, for Orca set to true
#define INVERT_E0_DIR false   // for direct drive extruder v9 set to true, for geared extruder set to false
#define INVERT_E1_DIR true    // for direct drive extruder v9 set to true, for geared extruder set to false
#define INVERT_E2_DIR false   // for direct drive extruder v9 set to true, for geared extruder set to false

// ENDSTOP SETTINGS:
// Sets direction of endstops when homing; 1=MAX, -1=MIN
#define X_HOME_DIR -1
#define Y_HOME_DIR -1
#define Z_HOME_DIR -1


#define min_software_endstops true // If true, axis won't move to coordinates less than HOME_POS.
#define max_software_endstops true  // If true, axis won't move to coordinates greater than the defined lengths below.

// Travel limits after homing
#define X_MIN_POS 0
#define Y_MIN_POS 0
#define Z_MIN_POS 0
#define X_MAX_POS 0
#define Y_MAX_POS 0
#define Z_MAX_POS 0

//#define X_MAX_LENGTH (X_MAX_POS - X_MIN_POS)
//#define Y_MAX_LENGTH (Y_MAX_POS - Y_MIN_POS)
//#define Z_MAX_LENGTH (Z_MAX_POS - Z_MIN_POS)

#define X_MAX_LENGTH 0
#define Y_MAX_LENGTH 0
#define Z_MAX_LENGTH 0

//#define HEATER_0_MAXTEMP_TRIG ( HEATER_0_MAXTEMP + 15 )
//#define HEATER_1_MAXTEMP_TRIG ( HEATER_1_MAXTEMP + 15 )
//#define HEATER_2_MAXTEMP_TRIG ( HEATER_2_MAXTEMP + 15 )
//#if (TEMP_SENSOR_BED == 1)
//  #define BED_MAXTEMP_TRIG      ( BED_MAXTEMP      + 10 )
//#endif

//============================= Bed Auto Leveling ===========================

//Manual homing switch locations:
// For deltabots this means top and center of the Cartesian print volume.
#define MANUAL_X_HOME_POS 0
#define MANUAL_Y_HOME_POS 0
#define MANUAL_Z_HOME_POS 0

#define HOMING_FEEDRATE_EB 0,0

#define HOMING_FEEDRATE {50*60, 50*60, 4*60, 0}
#define HOMING_FEEDRATE_X {50*60, 50*60, 4*60, HOMING_FEEDRATE_EB}

// 理论脉冲当量计算结果 360/1.8*16/(3.14159*11)=92.5993
// 直径为11mmE轴齿轮 脉冲当量为92，测试3台机器，进丝100mm，实际进丝96-97mm，取平均值96.5，
// 算出比例值为1.03627 -->> 计算出实际脉冲当量为92/1.03627=88.78
#define E_AXIS_STEPS_PER_UNIT_MM11  88.78f

// 理论脉冲当量计算结果 360/1.8*16/(3.14159*9)=113.1769
// 直径为11mmE轴齿轮 脉冲当量为111.93，进丝100mm，实际进丝95mm左右
// 算出比例值为1.05263 -->> 计算出实际脉冲当量为360/1.8*16/(3.14159*9)/1.05263=106.3367
#define E_AXIS_STEPS_PER_UNIT_MM9  106.3367f

// 文斌混色K5机型测试
// 理论脉冲当量计算结果 360/1.8*16/(3.14159*7.4)=137.64752
// 算出比例值为1.05263 -->> 计算出实际脉冲当量为360/1.8*16/(3.14159*7.4)/1.05263=130.76533
#define E_AXIS_STEPS_PER_UNIT_MM7_4  982.37f    //1018.591636f

// XY电机同步率20个齿，齿距2.03mm，计算得出脉冲当量为 360/1.8*16/(20 * 2.032)=78.7402
#define XY_AXIS_STEPS_PER_UNIT_MXL 78.7402f

// XY电机同步率20个齿，齿距2mm，计算得出脉冲当量为 360/1.8*16/(20 * 2)=80.0
#define XY_AXIS_STEPS_PER_UNIT_2GT 80.0f

// 3.0软料 24齿 1.17齿距 1.03 文斌 M3145D机器
#define E_AXIS_STEPS_PER_UNIT_24_117 110.6409f

#define DEFAULT_AXIS_STEPS_PER_UNIT_KELI   {32,32,640,96*20.0,0} // 颗粒机配置

// default steps per unit for Ultimaker
#define DEFAULT_AXIS_STEPS_PER_UNIT   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM11,0.0f}
#define DEFAULT_MAX_FEEDRATE          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION      {3000,3000,100,10000,0}    // X, Y, Z, E maximum start speed for accelerated moves. E default values are good for skeinforge 40+, for older versions raise them a lot.
// M2030
#define DEFAULT_AXIS_STEPS_PER_UNIT_M2030   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM11,0.0f}
#define DEFAULT_MAX_FEEDRATE_M2030          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M2030      {3000,3000,100,10000,0} 
// M2048
#define DEFAULT_AXIS_STEPS_PER_UNIT_M2048   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM11,0.0f}
#define DEFAULT_MAX_FEEDRATE_M2048          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M2048      {3000,3000,100,10000,0} 
// M3145
#define DEFAULT_AXIS_STEPS_PER_UNIT_M3145   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM11,0.0f}
#define DEFAULT_MAX_FEEDRATE_M3145          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M3145      {3000,3000,100,10000,0} 
// M4141
#define DEFAULT_AXIS_STEPS_PER_UNIT_M4141   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM11,0.0f}
#define DEFAULT_MAX_FEEDRATE_M4141          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M4141      {3000,3000,100,10000,0} 
//K5
#define DEFAULT_AXIS_STEPS_PER_UNIT_K5   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8,92.5993f,0.0f}
#define DEFAULT_MAX_FEEDRATE_K5          {80, 80, 5, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_K5      {250,250,50,2000,0}    // X, Y, Z, E maximum start speed for accelerated moves. E default values are good for skeinforge 40+, for older versions raise them a lot.
//3145s XY轴2GT 20齿 80.00脉冲当量；XY轴MXL 20齿 78.82脉冲当量http://www.ifmy.net/RepRapCalculator3.htm
// M3145S 2018.04.04统一为MXL同步轮带
#define DEFAULT_AXIS_STEPS_PER_UNIT_M3145S   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16.0f)/8.0f,E_AXIS_STEPS_PER_UNIT_MM9,0.0f}
#define DEFAULT_MAX_FEEDRATE_M3145S          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M3145S      {3000,3000,100,10000,0}
//M3145K
#define DEFAULT_AXIS_STEPS_PER_UNIT_M3145K   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16.0f)/8.0f,E_AXIS_STEPS_PER_UNIT_MM9,0.0f}
#define DEFAULT_MAX_FEEDRATE_M3145K          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M3145K      {3000,3000,100,10000,0}
//M2030HY
#define DEFAULT_AXIS_STEPS_PER_UNIT_M2030HY   {XY_AXIS_STEPS_PER_UNIT_2GT,XY_AXIS_STEPS_PER_UNIT_2GT,/*cs_step*/(200.0f*16.0f)/8.0f,E_AXIS_STEPS_PER_UNIT_MM9,0.0f}
#define DEFAULT_MAX_FEEDRATE_M2030HY          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M2030HY      {3000,3000,100,10000,0}
//M14S
#define DEFAULT_AXIS_STEPS_PER_UNIT_M14S {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16.0f)/8.0f,E_AXIS_STEPS_PER_UNIT_MM11,0.0f}
#define DEFAULT_MAX_FEEDRATE_M14S          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M14S      {3000,3000,100,10000,0}
/*20170523 9.1f*/
//M14R03
#define DEFAULT_AXIS_STEPS_PER_UNIT_M14R03   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM9,0.0f}
#define DEFAULT_MAX_FEEDRATE_M14R03          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M14R03      {3000,3000,100,10000,0}
//M41S
#define DEFAULT_AXIS_STEPS_PER_UNIT_M41S   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM11,0.0f}
#define DEFAULT_MAX_FEEDRATE_M41S          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M41S      {3000,3000,100,10000,0}
//AMP410W
#define DEFAULT_AXIS_STEPS_PER_UNIT_AMP410W   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM11,0.0f}
#define DEFAULT_MAX_FEEDRATE_AMP410W          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_AMP410W      {3000,3000,100,10000,0}
//M14
#define DEFAULT_AXIS_STEPS_PER_UNIT_M14   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8.0f,(/*cs_step*/(200.0f*16)/8.0f)/3.14159f,0.0f}
#define DEFAULT_MAX_FEEDRATE_M14          {300, 300, 10, 25,0}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M14      {3000,3000,100,10000,0}

// 文斌M41G机器
// XY电机同步轮20个齿，光轴同步轮22齿，齿距2.03mm MXL 3200/20/2.032 = 71.5820
#define DEFAULT_AXIS_STEPS_PER_UNIT_M41G   {XY_AXIS_STEPS_PER_UNIT_MXL, XY_AXIS_STEPS_PER_UNIT_MXL, /*cs_step*/(200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM9,0.0f}
#define DEFAULT_MAX_FEEDRATE_M41G        {150, 150, 10, 25, 25}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M41G     {3000,3000,100,10000,0}

// 梁海衡M4141S_NEW机器
// 电机20个齿带带到80个齿，同步带齿轮22个齿（与80个齿同光轴）
// 360 / 1.8 * 16 * 4 / (2.032 * 22) = 286.3278
#define DEFAULT_AXIS_STEPS_PER_UNIT_M41S_NEW {286.3278f, 286.3278f, (200.0f*16)/8.0f,E_AXIS_STEPS_PER_UNIT_MM11,0.0f}
#define DEFAULT_MAX_FEEDRATE_M41S_NEW        {150, 150, 10, 25, 25}//{500, 500, 5, 25}    // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M41S_NEW      {3000,3000,100,10000,0}

//M2030X
#define DEFAULT_AXIS_STEPS_PER_UNIT_M2030X   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8,E_AXIS_STEPS_PER_UNIT_MM9,E_AXIS_STEPS_PER_UNIT_MM9}
#define DEFAULT_MAX_FEEDRATE_M2030X          {300, 300, 10, 25, 25} // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M2030X      {3000,3000,100,10000,10000} 
//M2048X
#define DEFAULT_AXIS_STEPS_PER_UNIT_M2048X   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8,E_AXIS_STEPS_PER_UNIT_MM9,E_AXIS_STEPS_PER_UNIT_MM9}
#define DEFAULT_MAX_FEEDRATE_M2048X          {300, 300, 10, 25, 25} // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_M2048X      {3000,3000,100,10000,10000} 
//X5
#define DEFAULT_AXIS_STEPS_PER_UNIT_X5   {XY_AXIS_STEPS_PER_UNIT_MXL,XY_AXIS_STEPS_PER_UNIT_MXL,/*cs_step*/(200.0f*16)/8,E_AXIS_STEPS_PER_UNIT_MM7_4,E_AXIS_STEPS_PER_UNIT_MM7_4}
#define DEFAULT_MAX_FEEDRATE_X5          {80, 80, 5, 25, 25} // (mm/sec)
#define DEFAULT_MAX_ACCELERATION_X5      {250,250,50,2000,2000} 

//SOFT_filament
#define E_AXIS_STEPS_PER_UNIT_SOFT E_AXIS_STEPS_PER_UNIT_MM9
#define B_AXIS_STEPS_PER_UNIT_SOFT E_AXIS_STEPS_PER_UNIT_MM9


//周长算法: 1)齿数*齿距（此种算法算出的周长会稍微偏小）   2)2πr （建议采用此种算法）

////XY轴电机同步轮直径12.33mm   Z轴丝杆螺距是8mm  E轴齿轮直径11.0mm
////电机转一圈需要3200个脉冲(16细分)，对于XY轴一圈是12.33*π mm，对于Z轴一圈是8mm,对于XY轴一圈是11.0*π mm
//#define DEFAULT_AXIS_STEPS_PER_UNIT   {(/*cs_step*/200.0f*16.0f/12.33f)/3.14159f,(/*cs_step*/200.0f*16.0f/12.33f)/3.14159f,/*cs_step*/200.0f*16.0f/8.0f,(/*cs_step*/200.0f*16/11.0f)/3.14159f,0.0f} // default steps per unit for Ultimaker
//#define DEFAULT_MAX_FEEDRATE          {500, 500, 10, 45,0}// (mm/sec)
//#define DEFAULT_MAX_ACCELERATION      {9000,9000,100,10000,0}    // X, Y, Z, E maximum start speed for accelerated moves. E default values are good for skeinforge 40+, for older versions raise them a lot.

//XY轴电机同步轮直径12mm   Z轴丝杆螺距是8mm  E轴齿轮直径9.0mm
//电机转一圈需要3200个脉冲(16细分)，对于XY轴一圈是12*π mm，对于Z轴一圈是8mm,对于XY轴一圈是9.0*π mm
//#define DEFAULT_AXIS_STEPS_PER_UNIT_M2030HY   {(/*cs_step*/200.0f*16.0f/12.0f)/3.14159f,(/*cs_step*/200.0f*16.0f/12.0f)/3.14159f,/*cs_step*/200.0f*16.0f/8.0f,(/*cs_step*/200.0f*16/9.0f)/3.14159f,0.0f} // default steps per unit for Ultimaker

////COLOR_MIXING   //XY轴电机同步轮直径12.33mm   Z轴丝杆螺距是8mm  E轴齿轮直径9.0mm
////电机转一圈需要3200个脉冲(16细分)，对于XY轴一圈是12.33*π mm，对于Z轴一圈是8mm,对于XY轴一圈是9.0*π mm
//#define DEFAULT_AXIS_STEPS_PER_UNIT_X   {(/*cs_step*/200.0f*16.0f/12.33f)/3.14159f,(/*cs_step*/200.0f*16.0f/12.33f)/3.14159f,/*cs_step*/200.0f*16.0f/8.0f,(/*cs_step*/200.0f*16/9.0f)/3.14159f,(/*cs_step*/200.0f*16/9.0f)/3.14159f}//{78.7402,78.7402,200.0*8/3,760*1.1}   // default steps per unit for Ultimaker
//#define DEFAULT_MAX_FEEDRATE_X          {500, 500, 10, 45, 45}//{500, 500, 5, 25}    // (mm/sec)
//#define DEFAULT_MAX_ACCELERATION_X      {9000,9000,100,10000,10000}    // X, Y, Z, E maximum start speed for accelerated moves. E default values are good for skeinforge 40+, for older versions raise them a lot.


#define DEFAULT_ACCELERATION          600    // X, Y, Z and E max acceleration in mm/s^2 for printing moves
#define DEFAULT_RETRACT_ACCELERATION  1000   // X, Y, Z and E max acceleration in mm/s^2 for retracts

// The speed change that does not require acceleration (i.e. the software might assume it can be done instantaneously)
#define DEFAULT_XYJERK                20.0    // (mm/sec)
#define DEFAULT_ZJERK                 0.4     // (mm/sec)
#define DEFAULT_EJERK                 5.0    // (mm/sec)
#define DEFAULT_BJERK                 5.0    // (mm/sec) /*COLOR_MIXING*/
//===========================================================================
//=============================Additional Features===========================
//===========================================================================

// Preheat Constants
#define PLA_PREHEAT_HOTEND_TEMP 200//170
#define PLA_PREHEAT_HPB_TEMP 70


#define ABS_PREHEAT_HOTEND_TEMP 230//240
#define ABS_PREHEAT_HPB_TEMP 100



// Incrementing this by 1 will double the software PWM frequency,
// affecting heaters, and the fan if FAN_SOFT_PWM is enabled.
// However, control resolution will be halved for each increment;
// at zero value, there are 128 effective control positions.
#define SOFT_PWM_SCALE 0

#include "Configuration_adv.h"
//#include "thermistortables.h"

#define POWEROFF_UPDOWN_MINMIN //断电上下限位均为MIN PIN


#endif //__CONFIGURATION_H
