/// 
/// \file       CSampleRateConv.h
/// 
///             Defines audio class to "re-sample" an audio sample buffer
///


#ifndef SAMPLE_RATE_CONV_H
#define SAMPLE_RATE_CONV_H


#include <vector>
#include <iostream>

extern "C"
{
	#include "AudioConv.h"
}


template <typename T>
class CSampleRateConv
{
	
	double		m_dConversionRatio;

	// Lagrange 4-point interpolation (4th order interpolation)
	inline double Lagrange4Point(double y0, double y1, double y2, double y3, double mu) 
	{
		double a0, a1, a2, a3;
		
		a0 = (-0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3);
		a1 = (y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3);
		a2 = (-0.5 * y0 + 0.5 * y2);
		a3 = y1;
		
		// Horner's method for polynomial evaluation
		return ((a0 * mu + a1) * mu + a2) * mu + a3;
	}

public:

	CSampleRateConv()
	{
		m_dConversionRatio = 1;
	}
	
	bool setConversionRatio(double dRatio)
	{
		if (dRatio <= 0)
		{
			return false;
		}
		
		m_dConversionRatio = dRatio;
		
		return true;
	}
		
	bool setConversionRatio(unsigned int nInputSampleRate, unsigned int nOutputSampleRate)
	{
		if (nInputSampleRate == 0 || nOutputSampleRate == 0)
		{
			return false;
		}
		
		m_dConversionRatio = ((double) nOutputSampleRate / (double) nInputSampleRate);
		
		return true;
	}

	int resampleBuffer(const std::vector<T>& input, std::vector<T> &output) 
	{
		if (m_dConversionRatio <= 0)
		{
			return -1;
		}
		
		auto nInputLen = input.size();
		
		if (nInputLen > output.size())
		{
			output.resize(input.size());
		}
		
		double time = 0.0;
		
		while (time < nInputLen - 2) 
		{
			int index = static_cast<int>(time);
			
			double mu = time - index; // Fractional part

			T newSample = 0;
			
			// Ensure index allows 4 points (2 before, 2 after)
			if (index > 0 && index < (nInputLen - 2))
			{
				if constexpr (std::is_same_v<T, short>)
				{
					float fSample1 = audio16bitToFloat(input[index - 1]);
					float fSample2 = audio16bitToFloat(input[index]);
					float fSample3 = audio16bitToFloat(input[index + 1]);
					float fSample4 = audio16bitToFloat(input[index + 2]);

					float floatSample = (float)
						Lagrange4Point
						(
							(double) fSample1,
							(double) fSample2,
							(double) fSample3,
							(double) fSample4,
							mu
						);

					newSample = audioFloatTo16bit(floatSample);
				}
				else if constexpr (std::is_same_v<T, long>)
				{
					float fSample1 = audio32bitToFloat(input[index - 1]);
					float fSample2 = audio32bitToFloat(input[index]);
					float fSample3 = audio32bitToFloat(input[index + 1]);
					float fSample4 = audio32bitToFloat(input[index + 2]);

					float floatSample = (float)
						Lagrange4Point
						(
							(double) fSample1,
							(double) fSample2,
							(double) fSample3,
							(double) fSample4,
							mu
						);

					newSample = audioFloatTo32bit(floatSample);
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					newSample = (T)
						Lagrange4Point
						(
							(double) input[index - 1],
							(double) input[index],
							(double) input[index + 1],
							(double) input[index + 2],
							mu
						);
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					newSample = (T)
						Lagrange4Point
						(
							(double)input[index - 1],
							(double)input[index],
							(double)input[index + 1],
							(double)input[index + 2],
							mu
						);
				}
				else
				{
					// Data type not supported
					return -1;
				}
				
				output[index] = newSample;
			}
			
			// Step by the conversion ratio
			time += m_dConversionRatio; 	
		}
		
		if (nInputLen < output.size())
		{
			for (auto idx = nInputLen; idx < output.size(); idx++)
			{
				output[idx] = 0;
			}
		}			
		
		return nInputLen;
	}

	int resampleBuffer(const T *input, T *output, unsigned int nLen) 
	{
		if (m_dConversionRatio <= 0)
		{
			return -1;
		}
		
		if (nLen < 1)
		{
			return -1;
		}
		
		try
		{
			double time = 0.0;
			
			while (time < nLen - 2) 
			{
				int index = static_cast<int>(time);
				
				double mu = time - index; // Fractional part

				T newSample = 0;
				
				// Ensure index allows 4 points (2 before, 2 after)
				if (index > 0 && index < (nLen - 2))
				{
					if constexpr (std::is_same_v<T, short>)
					{
						float fSample1 = audio16bitToFloat(input[index - 1]);
						float fSample2 = audio16bitToFloat(input[index]);
						float fSample3 = audio16bitToFloat(input[index + 1]);
						float fSample4 = audio16bitToFloat(input[index + 2]);

						float floatSample = (float)
							Lagrange4Point
							(
								(double) fSample1,
								(double) fSample2,
								(double) fSample3,
								(double) fSample4,
								mu
							);

						newSample = audioFloatTo16bit(floatSample);
					}
					else if constexpr (std::is_same_v<T, long>)
					{
						float fSample1 = audio32bitToFloat(input[index - 1]);
						float fSample2 = audio32bitToFloat(input[index]);
						float fSample3 = audio32bitToFloat(input[index + 1]);
						float fSample4 = audio32bitToFloat(input[index + 2]);

						float floatSample = (float)
							Lagrange4Point
							(
								(double) fSample1,
								(double) fSample2,
								(double) fSample3,
								(double) fSample4,
								mu
							);

						newSample = audioFloatTo32bit(floatSample);
					}
					else if constexpr (std::is_same_v<T, float>)
					{
						newSample = (T)
							Lagrange4Point
							(
								(double) input[index - 1],
								(double) input[index],
								(double) input[index + 1],
								(double) input[index + 2],
								mu
							);
					}
					else if constexpr (std::is_same_v<T, double>)
					{
						newSample = (T)
							Lagrange4Point
							(
								(double) input[index - 1],
								(double) input[index],
								(double) input[index + 1],
								(double) input[index + 2],
								mu
							);
					}
					else
					{
						// Data type not supported
						return -1;
					}

					output[index] = newSample;
				}
				
				// Step by the conversion ratio
				time += m_dConversionRatio; 	
			}
					}
		catch(...)
		{
			return -1;
		}
		
		return nLen;
	}

};


#endif  //  SAMPLE_RATE_CONV_H
