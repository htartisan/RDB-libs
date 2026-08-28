/// 
/// \file       AudioUtils.cpp
/// 
///             Defines audio conversion utility functions
///


#include "AudioConv.h"



//inline int32_t audio16bitTo32bit(int16_t sample)
int32_t audio16bitTo32bit(int16_t sample)
{
    int32_t out = ((int32_t) sample) << 16;

    return out; 
}


//inline int16_t audio32bitTo16bit(int32_t sample)
int16_t audio32bitTo16bit(int32_t sample)
{
    int16_t out = (int16_t) (sample >> 16);

    return out; 
}


//inline float audio16bitToFloat(int16_t sample)
float audio16bitToFloat(int16_t sample)
{
    // Convert int16 value to float by scaling (dividing) the
    // int16 sample to the auto range of a float (+1.0 to -1.0)

    //double fSample = static_cast<double>(sample);
    float fSample = (float) sample;

    return (float) (fSample / MAX_16BIT_VALUE_AS_FLOAT);
}


//inline int16_t audioFloatTo16bit(float sample)
int16_t audioFloatTo16bit(float sample)
{
    // Convert float value to int16 by scaling (multipying) the
    // float sample to the range of a 16-bit integer (-32768 to 32767)

    float scaledSample = (((float) sample) * MAX_16BIT_VALUE_AS_FLOAT);

    //int16_t fixedSample = (static_cast<int16_t>(scaledSample));
    int16_t fixedSample = (int16_t) scaledSample;

    return fixedSample;
}


//inline float audio32bitToFloat(int32_t sample)
float audio32bitToFloat(int32_t sample)
{
    // Convert int32 value to float by scaling (dividing) the
    // int16 sample to the auto range of a float (+1.0 to -1.0)

    //double fSample = static_cast<double>(sample);
    float fSample = (float) sample;

    return (float) (fSample / MAX_32BIT_VALUE_AS_FLOAT);
}


//inline int32_t audioFloatTo32bit(float sample)
int16_t audioFloatTo32bit(float sample)
{
    // Convert float value to int16 by scaling (multipying) the
    // float sample to the range of a 32-bit integer (-2,147,483,647 to 2,147,483,647 )

    float scaledSample = (((float) sample) * MAX_32BIT_VALUE_AS_FLOAT);

    //int32_t fixedSample = (static_cast<int16_t>(scaledSample));
    int32_t fixedSample = (int32_t) scaledSample;

    return fixedSample;
}





