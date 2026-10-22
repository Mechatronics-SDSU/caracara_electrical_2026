/*
 * bitmask.h
 *
 *  Created on: Jul 11, 2026
 *      Author: tyoung
 */

#ifndef INC_BITMASK_H_
#define INC_BITMASK_H_

#include <stdint.h>

typedef struct SystemState SystemState;

void Bitmask_Init(SystemState *state);

uint8_t Bitmask_GetBit(uint32_t value, uint8_t index);

#endif /* INC_BITMASK_H_ */
