/*
 * bitmask.c
 *
 *  Created on: Jul 11, 2026
 *      Author: tyoung
 */

/**
 * @brief   Extracts a single bit from a 32-bit packed value.
 *
 * @param   value  32-bit variable containing a 14-bit packed bitfield (bits 0-13 in use).
 * @param   index  Bit position to read, using LSB-0 numbering (bit 0 = least significant / rightmost bit).
 *                 Expected to be a hardcoded constant in the range 0-13. No bounds checking is performed.
 *
 * @return  1 if the bit at `index` is set, 0 otherwise.
 */

#include <stdint.h>
#include "system_state.h"

static SystemState *systemState; //static pointer for globalState - pointer does not change, values do - but this is local scope decleration

void Bitmask_Init(SystemState *state){

	systemState = state;
}

uint8_t Bitmask_GetBit(uint32_t value, uint8_t index)
{
	uint32_t mask = 1UL << index; //left shit by the passed index to create the mask for the desired value.
	uint32_t result = ((value & (mask)) >> index);

    return (uint8_t)result;
}
