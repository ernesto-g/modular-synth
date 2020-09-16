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

#include <stdint.h>
#include "Ui.h"

#include "braids/settings.h"
#include "braids/drivers/adc.h"

#include "hal/Display.h"
#include "hal/Encoder.h"
#include "hal/Memory.h"


using namespace braids;

#define UI_STATE_SELECT_OSCILLATOR			0
#define UI_STATE_CONFIG_MENU				1
#define UI_STATE_SET_VALUE					2
#define UI_STATE_CALIB						3
#define UI_STATE_CALIB_STEP_2				4


struct S_OscData{
	uint8_t osc;
	uint8_t bank;
	int8_t symbol;
};
typedef struct S_OscData OscData;

static const OscData LIMITED_OSCILLATORS[OSCILLATOR_INDEX_LEN]={
		  {MACRO_OSC_SHAPE_CSAW,0,'0'},
		  {MACRO_OSC_SHAPE_MORPH,0,'1'},
		  {MACRO_OSC_SHAPE_SAW_SQUARE,0,'2'},
		  {MACRO_OSC_SHAPE_SINE_TRIANGLE,0,'3'},
		  {MACRO_OSC_SHAPE_BUZZ,0,'4'},
		  {MACRO_OSC_SHAPE_SQUARE_SUB,0,'5'},
		  {MACRO_OSC_SHAPE_SAW_SUB,0,'6'},
		  {MACRO_OSC_SHAPE_SQUARE_SYNC,0,'7'},
		  {MACRO_OSC_SHAPE_SAW_SYNC,0,'8'},
		  {MACRO_OSC_SHAPE_TRIPLE_SAW,0,'9'},
		  {MACRO_OSC_SHAPE_TRIPLE_SQUARE,0,'A'},
		  {MACRO_OSC_SHAPE_TRIPLE_TRIANGLE,0,'B'},
		  {MACRO_OSC_SHAPE_TRIPLE_SINE,0,'C'},
		  {MACRO_OSC_SHAPE_TRIPLE_RING_MOD,0,'D'},
		  {MACRO_OSC_SHAPE_SAW_SWARM,0,'E'},
		  {MACRO_OSC_SHAPE_SAW_COMB,0,'F'},
		  {MACRO_OSC_SHAPE_TOY,0,'G'},
		  {MACRO_OSC_SHAPE_DIGITAL_FILTER_LP,0,'H'},
		  {MACRO_OSC_SHAPE_DIGITAL_FILTER_PK,0,'I'},
		  {MACRO_OSC_SHAPE_DIGITAL_FILTER_BP,0,'J'},

		  {MACRO_OSC_SHAPE_DIGITAL_FILTER_HP,1,'0'},
		  {MACRO_OSC_SHAPE_VOSIM,1,'1'},
		  {MACRO_OSC_SHAPE_VOWEL,1,'2'},
		  {MACRO_OSC_SHAPE_VOWEL_FOF,1,'3'},
		  {MACRO_OSC_SHAPE_HARMONICS,1,'4'},
		  {MACRO_OSC_SHAPE_FM,1,'5'},
		  {MACRO_OSC_SHAPE_FEEDBACK_FM,1,'6'},
		  {MACRO_OSC_SHAPE_CHAOTIC_FEEDBACK_FM,1,'7'},
		  {MACRO_OSC_SHAPE_PLUCKED,1,'8'},
		  {MACRO_OSC_SHAPE_BOWED,1,'9'},
		  {MACRO_OSC_SHAPE_BLOWN,1,'A'},
		  {MACRO_OSC_SHAPE_FLUTED,1,'B'},
		  {MACRO_OSC_SHAPE_STRUCK_BELL,1,'C'},
		  {MACRO_OSC_SHAPE_STRUCK_DRUM,1,'D'},
		  {MACRO_OSC_SHAPE_KICK,1,'E'},
		  {MACRO_OSC_SHAPE_CYMBAL,1,'F'},
		  {MACRO_OSC_SHAPE_SNARE,1,'G'},
		  {MACRO_OSC_SHAPE_FILTERED_NOISE,1,'H'},
		  {MACRO_OSC_SHAPE_TWIN_PEAKS_NOISE,1,'I'},
		  {MACRO_OSC_SHAPE_CLOCKED_NOISE,1,'J'},
		  {MACRO_OSC_SHAPE_GRANULAR_CLOUD,1,'L'},
		  {MACRO_OSC_SHAPE_PARTICLE_NOISE,1,'O'},
		  {MACRO_OSC_SHAPE_DIGITAL_MODULATION,1,'P'}
			};

struct S_ConfigItem{
	int8_t symbol;
	Setting option;
};
typedef struct S_ConfigItem ConfigItem;
#define CONFIG_ITEMS_SYMBOLS_LEN	11
static const ConfigItem CONFIG_ITEMS[CONFIG_ITEMS_SYMBOLS_LEN]={
		{'A',SETTING_AD_ATTACK}, // Attack
		{'D',SETTING_AD_DECAY}, // Decay
		{'F',SETTING_AD_FM}, // FM Mod
		{'T',SETTING_AD_TIMBRE}, // Timbre Mod
		{'C',SETTING_AD_COLOR}, // Color Mod
		{'H',SETTING_AD_VCA}, // VCA Mod
		{'E',SETTING_META_MODULATION}, // Meta on off
		{'G',SETTING_TRIG_SOURCE}, // Trigger auto on off
		{'U',SETTING_QUANTIZER_SCALE}, // Quantizer Off Tone Semitone
		{'R',SETTING_QUANTIZER_ROOT}, // Root
		{'L',SETTING_VCO_FLATTEN} // Flat On OFF
		//{'O',SETTING_VCO_DRIFT} // Drift on off . NOT IMPLEMENTED (bc jitter is not implemented)
};


void Ui::init(Adc* adc,Memory* memory) {
	display.init();
	encoder.Init();
	state = UI_STATE_SELECT_OSCILLATOR;
	timeoutCalibCounter=0;
	enterCalibCounter=0;
	this->adc = adc;
	this->memory = memory;

	this->flagUncal=2;
	this->fineTuneChanged=0;
	this->oscillatorChanged=1;
	this->flagJustFinishedRender=0;
	this->flagPendingSetCurrentOscillator=0;

	// Load current oscillator from eeprom
	currentOscillator=(int8_t)memory->readUInt8(Memory::ADDR_CURRENT_OSCILLATOR);
	if(currentOscillator>=OSCILLATOR_INDEX_LEN)
	{
		currentOscillator=0;
	}
	display.showChar(LIMITED_OSCILLATORS[currentOscillator].symbol);
	display.showBank(LIMITED_OSCILLATORS[currentOscillator].bank);
	settings.SetValue(SETTING_OSCILLATOR_SHAPE, LIMITED_OSCILLATORS[currentOscillator].osc);
	display.showConfig(0);
	//________________________

	// Load all settings from eeprom
	loadAllSettings();

}

int16_t Ui::validateOscillatorIndex(int16_t index)
{
	if(index>=OSCILLATOR_INDEX_LEN)
	{
		return OSCILLATOR_INDEX_LEN-1;
	}
	if(index<0)
		return 0;

	return index;
}

void Ui::showMetaOscillator(int16_t index)
{
	uint8_t i = validateOscillatorIndex(index);

	display.showChar(LIMITED_OSCILLATORS[i].symbol);
	display.showBank(LIMITED_OSCILLATORS[i].bank);
}

MacroOscillatorShape Ui::getOscillatorShapeFromIndex(int16_t index)
{
	uint8_t i = validateOscillatorIndex(index);

	MacroOscillatorShape osc_shape = static_cast<MacroOscillatorShape>(LIMITED_OSCILLATORS[i].osc);

	return osc_shape;
}

void Ui::showUnCalibrated(void)
{
	display.showDigitDot();
	if(flagUncal!=1)
	{
		flagUncal=1;
		fineTuneChanged=1;
	}
}
void Ui::showCalibrated(void)
{
	display.hideDigitDot();
	if(flagUncal!=0)
	{
		flagUncal=0;
		fineTuneChanged=1;
	}
}

void Ui::sysTick(void)
{
	encoder.sysTick();

	if(timeoutCalibCounter>0)
	{
		timeoutCalibCounter--;
		if(timeoutCalibCounter==0)
		{
			enterCalibCounter=0;
		}
	}

}

void Ui::saveAllSettings(void)
{
	uint16_t i;
	for(i=0; i<CONFIG_ITEMS_SYMBOLS_LEN; i++)
	{
		Setting setting_ = CONFIG_ITEMS[i].option;
		int16_t value = settings.GetValue(setting_);
		this->memory->writeUInt16(Memory::ADDR_BASE_SETTINGS+(i*(sizeof(uint16_t))), (uint16_t)value);
	}
}
void Ui::loadAllSettings(void)
{
	uint16_t i;
	for(i=0; i<CONFIG_ITEMS_SYMBOLS_LEN; i++)
	{
		Setting setting_ = CONFIG_ITEMS[i].option;

		int16_t valMem = (int16_t)this->memory->readUInt16(Memory::ADDR_BASE_SETTINGS+(i*(sizeof(uint16_t))));

		int16_t value = settings.metadata(setting_).Clip(valMem);
		settings.SetValue(setting_, value);
	}
}

void Ui::justFinishedRender(void)
{
	flagJustFinishedRender=1;
}

void Ui::loop(void) {

	encoder.loop();

	switch(state)
	{
		case UI_STATE_SELECT_OSCILLATOR:
		{
			if(flagJustFinishedRender)
			{
				flagJustFinishedRender=0;
				if(fineTuneChanged)
				{
					fineTuneChanged=0;
					display.showChar(LIMITED_OSCILLATORS[currentOscillator].symbol); // update display to show DOT
				}
				if(oscillatorChanged)
				{
					oscillatorChanged=0;
					display.showChar(LIMITED_OSCILLATORS[currentOscillator].symbol);
					display.showBank(LIMITED_OSCILLATORS[currentOscillator].bank);

				}
				if(flagPendingSetCurrentOscillator)
				{
					flagPendingSetCurrentOscillator=0;
					settings.SetValue(SETTING_OSCILLATOR_SHAPE, LIMITED_OSCILLATORS[currentOscillator].osc);
					display.showConfig(0);
					memory->writeUInt8NoWait(Memory::ADDR_CURRENT_OSCILLATOR, (uint8_t)currentOscillator);
				}
			}

			int32_t increment = encoder.increment();
			if (increment != 0)
			{
				if(increment>0){
					currentOscillator++;
				}
				else{
					currentOscillator--;
				}

				if(currentOscillator>=OSCILLATOR_INDEX_LEN)
					currentOscillator=(OSCILLATOR_INDEX_LEN-1);
				if(currentOscillator<0)
					currentOscillator=0;

				oscillatorChanged=1; // to update display later
				display.showConfig(1);
			}
			if(encoder.pressed())
			{
				flagPendingSetCurrentOscillator=1;
			}

			if(encoder.pressedLong())
			{
				state = UI_STATE_CONFIG_MENU;
				display.showConfig(1);
				display.showBank(2); // both off
				configIndex=0;
				enterCalibCounter=0;
			}



			break;
		}
		case UI_STATE_CONFIG_MENU:
		{
			if(encoder.pressedLong())
			{
				state = UI_STATE_SELECT_OSCILLATOR;
				display.showConfig(0);
				display.showBank(LIMITED_OSCILLATORS[currentOscillator].bank);
				display.showChar(LIMITED_OSCILLATORS[currentOscillator].symbol);

				// save all settings in EEPROM
				saveAllSettings();
				break;
			}

			display.showChar(CONFIG_ITEMS[configIndex].symbol);

			int32_t increment = encoder.increment();
			if (increment != 0)
			{
				if(increment>0)
					configIndex++;
				else
					configIndex--;

				if(configIndex>=CONFIG_ITEMS_SYMBOLS_LEN)
					configIndex=CONFIG_ITEMS_SYMBOLS_LEN-1;
				if(configIndex<0)
					configIndex=0;
			}

			if(encoder.pressed())
			{
				// Enter to option
				state = UI_STATE_SET_VALUE;

				// Calibration mode detection
				if(enterCalibCounter==0)
					timeoutCalibCounter=2000;

				enterCalibCounter++;
				if(enterCalibCounter>=4)
				{
					enterCalibCounter=0;
					state = UI_STATE_CALIB;
					display.showConfig(1);
					display.showBank(3); // all leds ON
					display.showChar('2');
				}
				//___________________________
			}

			break;
		}
		case UI_STATE_SET_VALUE:
		{
			Setting setting_ = CONFIG_ITEMS[configIndex].option;
			int16_t value = settings.GetValue(setting_);

			int32_t increment = encoder.increment();
			if (increment != 0)
			{
				value = settings.metadata(setting_).Clip(value + increment);
				settings.SetValue(setting_, value);
			}

			// show value
			display.showValue(settings.metadata(setting_).max_value,value);

			if(encoder.pressed())
			{
				// go back to config menu
				state = UI_STATE_CONFIG_MENU;
			}
			break;
		}
		case UI_STATE_CALIB:
		{
			if(encoder.pressedLong())
			{
				state = UI_STATE_CONFIG_MENU;
				// go back to config menu
				display.showConfig(1);
				display.showBank(2); // all leds OFF
				break;
			}

			if(encoder.pressed())
			{
				// save CV values and go to step 2
				adc_code_c2 = adc->channel(ADC_CHANNEL_PITCH);
				adc_code_p0_min = adc->channel(ADC_CHANNEL_PARAM0);
				adc_code_p1_min = adc->channel(ADC_CHANNEL_PARAM1);

				state = UI_STATE_CALIB_STEP_2;
				display.showChar('4');
			}
			break;
		}
		case UI_STATE_CALIB_STEP_2:
		{
			if(encoder.pressedLong())
			{
				state = UI_STATE_CONFIG_MENU;
				// go back to config menu
				display.showConfig(1);
				display.showBank(2); // all leds OFF
				break;
			}

			if(encoder.pressed())
			{
				int32_t adc_code_c4 = adc->channel(ADC_CHANNEL_PITCH);
				int32_t adc_code_p0_max = adc->channel(ADC_CHANNEL_PARAM0);
				int32_t adc_code_p1_max = adc->channel(ADC_CHANNEL_PARAM1);
				int32_t adc_code_fm=0; //adc->channel(ADC_CHANNEL_FINE_TUNE);

				settings.Calibrate(adc_code_c2,
						      adc_code_c4,
						      adc_code_fm,
						      adc_code_p0_min,
						      adc_code_p0_max,
						      adc_code_p1_min,
						      adc_code_p1_max);

				// Save in eeprom
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_C2, adc_code_c2);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_C4, adc_code_c4);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_FM, adc_code_fm);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_P0_MIN, adc_code_p0_min);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_P0_MAX, adc_code_p0_max);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_P1_MIN, adc_code_p1_min);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_P1_MAX, adc_code_p1_max);

				// go back to config menu
				state = UI_STATE_CONFIG_MENU;
			}
			break;
		}
	}

}

