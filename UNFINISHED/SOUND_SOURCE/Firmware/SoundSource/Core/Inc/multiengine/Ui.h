/*
 * Ui.h
 *
 *  Created on: Sep 8, 2020
 *      Author: ernesto
 */

#ifndef INC_MULTIENGINE_UI_H_
#define INC_MULTIENGINE_UI_H_

#include <stdint.h>
#include "hal/Display.h"
#include "hal/Encoder.h"
#include "braids/drivers/adc.h"
#include "hal/Memory.h"

using namespace braids;

class Ui
{
	public:
		void init(Adc* adc,Memory* memory);
		void loop(void);
		void sysTick(void);
	private:
		Display display;
		Encoder encoder;
		int8_t currentOscillator;
		uint8_t state;
		int8_t configIndex;
		int16_t timeoutCalibCounter;
		uint8_t enterCalibCounter=0;
		Adc* adc;
		Memory* memory;
		int32_t adc_code_c2;
		int32_t adc_code_p0_min;
		int32_t adc_code_p1_min;
		void saveAllSettings(void);
		void loadAllSettings(void);
};

#endif /* INC_MULTIENGINE_UI_H_ */
