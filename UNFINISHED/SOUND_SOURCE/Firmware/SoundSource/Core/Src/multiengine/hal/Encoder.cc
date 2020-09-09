// Copyright 2012 Emilie Gillet.
// Copyright 2020 Ernesto Gigliotti
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
// Author: Ernesto Gigliotti (ernestogigliotti@gmail.com)
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
// Driver for rotary encoder.

#include <hal/Encoder.h>
#include "hal/MultiEngineHAL.h"

#define ENCODER_SW_STATE_IDLE			0
#define ENCODER_SW_STATE_WAIT_RELEASE 	1
#define ENCODER_SW_STATE_DEBOUNCE		2


void Encoder::Init() {
  switch_state_ = 0xff;
  quadrature_decoding_state_[0] = quadrature_decoding_state_[1] = 0xff;
  state = ENCODER_SW_STATE_IDLE;
}

void Encoder::sysTick(void) {
	if(swTimeout>0)
	{
		swTimeout--;
	}
}


void Encoder::loop(void) {
	switch(state)
	{
		case ENCODER_SW_STATE_IDLE:
		{
			if(mehal_readEncoderSwitch()==0)
			{
				state = ENCODER_SW_STATE_WAIT_RELEASE;
				swTimeout = 3000;
			}
			break;
		}
		case ENCODER_SW_STATE_WAIT_RELEASE:
		{
			if(swTimeout==0)
			{
				// long press
				flagLongPress=1;
				state = ENCODER_SW_STATE_DEBOUNCE;
				swTimeout = 100;
			}
			if(mehal_readEncoderSwitch()==1)
			{
				state = ENCODER_SW_STATE_DEBOUNCE;
				swTimeout = 100;
				flagShortPress=1;
			}
			break;
		}
		case ENCODER_SW_STATE_DEBOUNCE:
		{
			if(mehal_readEncoderSwitch()==1)
			{
				if(swTimeout==0)
				{
					state = ENCODER_SW_STATE_IDLE;
				}
			}
			break;
		}
	}

	// encoder reading
	  switch_state_ = (switch_state_ << 1) | \
			  mehal_readEncoderSwitch();

	  quadrature_decoding_state_[0] = (quadrature_decoding_state_[0] << 1) | \
			  mehal_readEncoderB();

	  quadrature_decoding_state_[1] = (quadrature_decoding_state_[1] << 1) | \
			  mehal_readEncoderA();
	//_________
}

bool Encoder::pressedLong(void)
{
	bool r = flagLongPress;
	if(r!=0)
		flagLongPress=0;

	return r;
}

bool Encoder::pressed(void) {
    //return switch_state_ == 0x00;
	bool r = flagShortPress;
	flagShortPress=0;
	return r;
  }


