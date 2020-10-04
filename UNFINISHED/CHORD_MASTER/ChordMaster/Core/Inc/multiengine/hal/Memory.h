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

#ifndef INC_MULTIENGINE_HAL_MEMORY_H_
#define INC_MULTIENGINE_HAL_MEMORY_H_


class Memory{
	public:
		uint32_t readUInt32(uint16_t address);
		uint8_t writeUInt32(uint16_t address,uint32_t value);

		uint8_t readUInt8(uint16_t address);
		uint8_t writeUInt8(uint16_t address,uint8_t value);
		uint8_t writeUInt8NoWait(uint16_t address,uint8_t value);

		uint16_t readUInt16(uint16_t address);
		uint8_t writeUInt16(uint16_t address,uint16_t value);

		const static uint16_t ADDR_ADC_CODE_C2=0;
		const static uint16_t ADDR_ADC_CODE_C4=4;
		const static uint16_t ADDR_ADC_CODE_FM=8;
		const static uint16_t ADDR_ADC_CODE_P0_MIN=12;
		const static uint16_t ADDR_ADC_CODE_P0_MAX=16;
		const static uint16_t ADDR_ADC_CODE_P1_MIN=20;
		const static uint16_t ADDR_ADC_CODE_P1_MAX=24;
		const static uint8_t ADDR_CURRENT_OSCILLATOR=28;
		// 29 to 31 free
		const static uint8_t ADDR_BASE_SETTINGS=32;

};


#endif /* INC_MULTIENGINE_HAL_MEMORY_H_ */
