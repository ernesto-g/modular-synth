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



class Ui
{
	public:
		void init(void);
		void loop(void);
		void sysTick(void);
	private:
		Display display;
		Encoder encoder;
		int8_t currentOscillator;
		uint8_t state;
		int8_t configIndex;
};

#endif /* INC_MULTIENGINE_UI_H_ */
