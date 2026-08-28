/// 
/// \file       AudioConv.h
/// 
///             Defines audio utility functions to convert samples
///


#ifndef AUDIO_CONV_H
#define AUDIO_CONV_H

#include <stdint.h>


#define MAX_16BIT_VALUE                 ((uint16_t) 32767)
#define MAX_16BIT_VALUE_AS_FLOAT        ((float) 32767.0)

#define MAX_32BIT_VALUE                 ((uint32_t) 2147483647)
#define MAX_32BIT_VALUE_AS_FLOAT        ((float) 2147483647.0)


//inline int32_t audio16bitTo32bit(int16_t sample);
int32_t audio16bitTo32bit(int16_t sample);

//inline int16_t audio32bitTo16bit(int32_t sample);
int16_t audio32bitTo16bit(int32_t sample);


//inline float audio16bitToFloat(int16_t sample);
float audio16bitToFloat(int16_t sample);

//inline int16_t audioFloatTo16bit(float sample);
int16_t audioFloatTo16bit(float sample);

//inline float audio32bitToFloat(int32_t sample);
float audio32bitToFloat(int32_t sample);

//inline int32_t audioFloatTo32bit(float sample);
int16_t audioFloatTo32bit(float sample);


#endif  //  AUDIO_CONV_H
