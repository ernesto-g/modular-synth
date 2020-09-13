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
#include "hal/MultiEngineHAL.h"
#include "hal/Memory.h"

uint32_t Memory::readUInt32(uint16_t address)
{
	uint32_t value;
	mehal_i2cMemRead(address,(uint8_t*)&value, sizeof(uint32_t));
	return value;
}
uint8_t Memory::readUInt8(uint16_t address)
{
	uint8_t value;
	mehal_i2cMemRead(address, &value, sizeof(uint8_t));
	return value;
}

uint16_t Memory::readUInt16(uint16_t address)
{
	uint16_t value;
	mehal_i2cMemRead(address, (uint8_t*)&value, sizeof(uint16_t));
	return value;
}

uint8_t Memory::writeUInt32(uint16_t address,uint32_t value)
{
	return mehal_i2cMemWrite(address, (uint8_t*)&value, sizeof(uint32_t));
}

uint8_t Memory::writeUInt8(uint16_t address,uint8_t value)
{
	return mehal_i2cMemWrite(address, &value, sizeof(uint8_t));
}

uint8_t Memory::writeUInt16(uint16_t address,uint16_t value)
{
	return mehal_i2cMemWrite(address, (uint8_t*)&value, sizeof(uint16_t));
}

