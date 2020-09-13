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

#ifndef INC_MULTIENGINE_MAINLOOP_H_
#define INC_MULTIENGINE_MAINLOOP_H_

#include <stdint.h>

class MainLoop
{
	public:
	void init(void);
	void loop(void);
	void render(uint8_t* out, uint32_t outSize);
	void adcStateMachine(void);
};




#endif /* INC_MULTIENGINE_MAINLOOP_H_ */
