/*
MIT License

Copyright (c) 2020 Ernesto Gigliotti
Copyright (c) 2018 Marcel Meyer-Garcia

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#ifndef INC_MULTIENGINE_HAL_MULTIENGINEHAL_H_
#define INC_MULTIENGINE_HAL_MULTIENGINEHAL_H_

#include <stdint.h>

extern "C" {
void mehal_init(uint32_t* samplesBuffer,uint32_t samplesBufferSize,void (*fnCallbackDMA)(int));
void mehal_toogleBoardLed(void);
void mehal_delay(uint32_t t);

uint16_t mehal_readADC( uint8_t channel);

void mehal_debugPinSet(void);
void mehal_debugPinReset(void);

uint8_t mehal_readGateInput(void);

uint8_t mehal_readEncoderSwitch(void);
uint8_t mehal_readEncoderA(void);
uint8_t mehal_readEncoderB(void);

void mehal_sendDataTo595(uint8_t data);

void mehal_setBank0Led(uint8_t state);
void mehal_setBank1Led(uint8_t state);
void mehal_setConfigLed(uint8_t state);

uint8_t mehal_i2cMemRead(uint16_t memAddress,uint8_t *pData, uint16_t size);
uint8_t mehal_i2cMemWrite(uint16_t memAddress,uint8_t *pData, uint16_t size,uint8_t flagWait);

}

#endif /* INC_MULTIENGINE_HAL_MULTIENGINEHAL_H_ */
