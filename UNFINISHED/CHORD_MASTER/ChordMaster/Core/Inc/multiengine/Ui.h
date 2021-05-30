/*
 *  Sound Source. Macro-oscillator using Blue Pill. Based on Mutable Instruments Braids.
    Copyright (C) 2020  Ernesto Gigliotti. ernestogigliotti@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INC_MULTIENGINE_UI_H_
#define INC_MULTIENGINE_UI_H_

#include <stdint.h>
#include "hal/Display.h"
#include "hal/Encoder.h"
#include "braids/drivers/adc.h"
#include "hal/Memory.h"
#include "braids/settings.h"


#define OSCILLATOR_INDEX_LEN	32

using namespace braids;

class Ui
{
	public:
		void init(Adc* adc,Memory* memory);
		void loop(void);
		void sysTick(void);
		int16_t validateOscillatorIndex(int16_t index);
		void showMetaOscillator(int16_t index);
		MacroOscillatorShape getOscillatorShapeFromIndex(int16_t index);
		void showUnCalibrated(void);
		void showCalibrated(void);
		void justFinishedRender(void);
		uint16_t getParamFromCurrentWaveTable(void);

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
		uint8_t flagUncal;
		uint8_t fineTuneChanged;
		uint8_t oscillatorChanged;
		uint8_t flagJustFinishedRender;
		uint8_t flagPendingWriteValueInEEPROM;
		void saveAllSettings(void);
		void loadAllSettings(void);
};

#endif /* INC_MULTIENGINE_UI_H_ */
