/*
 * MultiEngineHAL.h
 *
 *  Created on: Aug 13, 2020
 *      Author: ernesto
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

}

#endif /* INC_MULTIENGINE_HAL_MULTIENGINEHAL_H_ */
