/*
 * Memory.cpp
 *
 *  Created on: Sep 12, 2020
 *      Author: ernesto
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

