/*
 * Display.cpp
 *
 *  Created on: Sep 6, 2020
 *      Author: ernesto
 */

#include <stdint.h>
#include "hal/Display.h"
#include "hal/MultiEngineHAL.h"

static const int8_t NUMBERS_TABLE[]={
		0b01011111, // 0
		0b00000110, // 1
		0b00111011, // 2
		0b00101111, // 3
		0b01100110, // 4
		0b01101101, // 5
		0b01111101, // 6
		0b00000111, // 7
		0b01111111, // 8
		0b01100111  // 9
};

static const int8_t CHARS_TABLE[]={
		0b01110111, // A
		0b01111100, // B
		0b01011001, // C
		0b00111110, // D
		0b01111001, // E
		0b01110001, // F
		0b01101111, // G
		0b01110110, // H
		0b01010000, // I
		0b00011110, // J
		0b00000000, // K *
		0b01011000, // L
		0b00000000, // M *
		0b00000000, // N *
		0b00111100, // O
		0b01110011, // P
		0b00000000, // Q *
		0b00110000, // R
		0b00000000, // S *
		0b01111000, // T
		0b00011100, // U
		0b00000000, // V *
		0b00000000, // W *
		0b00000000, // X *
		0b00000000, // Y *
		0b00000000  // Z *
};

void Display::init(void) {

}

void Display::showChar(int8_t c) {

	uint8_t data;

	if(c>='A' && c<='Z')
		data = CHARS_TABLE[c - 'A'];
	else
		data = NUMBERS_TABLE[c - '0'];

	data = ~data;

	mehal_sendDataTo595(data);
}

void Display::showBank(uint8_t bankNumber) {

	switch(bankNumber)
	{
		case 0:
			mehal_setBank0Led(1);
			mehal_setBank1Led(0);
			break;
		case 1:
			mehal_setBank0Led(0);
			mehal_setBank1Led(1);
			break;
		case 2:
			mehal_setBank0Led(0);
			mehal_setBank1Led(0);
			break;
	}
}

void Display::showConfig(uint8_t flagOnOff) {

	mehal_setConfigLed(flagOnOff);
}

void Display::showValue(uint8_t maxValue,uint8_t value)
{
	switch(maxValue)
	{
		case 15:
		{
			// 0 to 15 scale
			uint16_t val = value*9/15;
			showChar('0'+val);
			break;
		}
		case 1:
		{
			// on off scale
			if(value==0)
				showChar('0');
			else
				showChar('1');
			break;
		}
		case 2:
		{
			// 3 values scale
			if(value==0)
				showChar('0');
			else if(value==1)
				showChar('1');
			else
				showChar('2');
			break;
		}
		case 4:
		case 11:
		case 19:
		{
			if(value<=9)
				showChar('0'+value);
			else
				showChar('A'+(value-10));

			break;
		}

	}
}

