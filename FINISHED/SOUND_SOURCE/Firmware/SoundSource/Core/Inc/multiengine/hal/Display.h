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

#ifndef INC_MULTIENGINE_DISPLAY_H_
#define INC_MULTIENGINE_DISPLAY_H_

#include <stdint.h>

class Display
{
	public:
	void init(void);
	void showChar(int8_t c);
	void showBank(uint8_t bankNumber);
	void showConfig(uint8_t flagOnOff);
	void showValue(uint8_t maxValue,uint8_t value);
	void showDigitDot(void);
	void hideDigitDot(void);
	private:
	uint8_t dotState;
};



#endif /* INC_MULTIENGINE_DISPLAY_H_ */
