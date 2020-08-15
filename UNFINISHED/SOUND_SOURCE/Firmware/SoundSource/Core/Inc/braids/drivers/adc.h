// Copyright 2012 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Driver for ADC scanning.

#ifndef BRAIDS_DRIVERS_ADC_H_
#define BRAIDS_DRIVERS_ADC_H_

#include <stdint.h>

namespace braids {

#define ADC_CHANNELS	5

#define ADC_CHANNEL_PARAM0		0
#define ADC_CHANNEL_PARAM1		1
#define ADC_CHANNEL_PITCH		2
#define ADC_CHANNEL_FM			3
#define ADC_CHANNEL_FINE_TUNE	4


class Adc {
 public:
  Adc() { }
  ~Adc() { }
  
  void Init(void);
  
  uint16_t channel(uint8_t index) const
  {
	  // 12 bit values

	  /*
	  if(index<ADC_CHANNELS)
	  {
		  return this->currentValues[index];
	  }
	  else
		  return 0;
		*/

	  // FOR DEBUGGING
	  if(index==0)
	  {
		  // param 1
		  return this->currentValues[index];
	  }
	  else if(index==1)
	  {
		  // param 2
		  return 2048; //this->currentValues[index];
	  }
	  else if(index==2)
	  {
		  ///pitch
		  return this->currentValues[index];
	  }
	  else if(index==3)
	  {
		  // FM
		  return 0 ; //this->currentValues[index];
	  }
	  else if(index==4)
	  {
		  // Fine tune
		  return 0;
	  }
	  else
		  return 2048;

  }

  void updateChannelValue(uint8_t chn,uint16_t value)
  {
	  if(chn<ADC_CHANNELS)
	  {
		  this->currentValues[chn] = value;
	  }
  }
 
 private:
  	  uint16_t currentValues[ADC_CHANNELS]={2048,2048,3000,0};
};

}  // namespace braids

#endif  // BRAIDS_DRIVERS_ADC_H_
