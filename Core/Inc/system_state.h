/*
 * system_state.h
 *
 *  Created on: Jun 4, 2026
 *      Author: Trevor Young
 */

#ifndef INC_SYSTEM_STATE_H_
#define INC_SYSTEM_STATE_H_

#include <stdbool.h>
#include <stdint.h>



/*
 * @file This struct is used to hold global data for all subsystems of caracara
 *
 */

//servo and dropper values will need to be added in the future

typedef struct SystemState{
	//USB commands from orin
	uint32_t motorValues[8]; //indices 0-7 correspond to motors 0-7
	uint32_t killState;
	uint32_t powerOff;
	uint32_t servo1;
	uint32_t servo2;
	uint32_t dropper;
	uint32_t torpedo;

	//Kill System State
	uint8_t intKillState;
	uint8_t extKillState;

	//powerDown
	uint32_t powerDownButton;

	//variables for LED to use to indicate system state
	uint8_t usbConnected;
	uint8_t motorsArming;

	//MS5837 depth sensor


}SystemState;

/* Parse a raw USB receive buffer into state.
 * Returns 0 on success, -1 if the packet is too short. */
int8_t SystemState_UpdateFromUSB(SystemState *state, const uint8_t *buffer, uint8_t len);

extern SystemState state;

#endif /* INC_SYSTEM_STATE_H_ */
