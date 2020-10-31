#ifndef treed_motion_h
#define treed_motion_h

#define F_CPU 16000000UL

//#define HIGH 0x1
//#define LOW  0x0

//#ifndef min
//#define min(a,b) ((a)<(b)?(a):(b))
//#endif // min

//#ifndef max
//#define max(a,b) ((a)>(b)?(a):(b))
//#endif // max

//#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define MAX_NUM_AXIS 5
#define XYZ_NUM_AXIS 3
enum AxisEnum {X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, E_AXIS = 3, B_AXIS = 4};

//===========================================================================
//=============================Buffers           ============================
//===========================================================================

// The number of linear motions that can be in the plan at any give time.
// THE BLOCK_BUFFER_SIZE NEEDS TO BE A POWER OF 2, i.g. 8,16,32 because shifts and ors are used to do the ring-buffering.
#define BLOCK_BUFFER_SIZE 16 // maximize block buffer

#endif



