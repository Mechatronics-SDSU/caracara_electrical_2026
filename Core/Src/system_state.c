/*
 * system_state.c
 *
 *  Created on: Jun 4, 2026
 *      Author: Trevor Young
 */

#include "system_state.h"
#include "main.h"
#include <string.h>

/* Expected USB packet layout from Orin - all values 4 bytes, little-endian format
 *
 *  Offset  Index  Field
 *  ------  -----  -----
 *   0      [0]    motorValues[0]
 *   4      [1]    motorValues[1]
 *   8      [2]    motorValues[2]
 *   12     [3]    motorValues[3]
 *   16     [4]    motorValues[4]
 *   20     [5]    motorValues[5]
 *   24     [6]    motorValues[6]
 *   28     [7]    motorValues[7]
 *   32     [8]    killState       (uint32_t, lower byte used)
 *   36     [9]    powerOff        (uint32_t, lower byte used)
 *   40     [10]   servo1          (uint32_t, little-endian)
 *   44     [11]   servo2          (uint32_t, little-endian)
 *   48     [12]   dropper         (uint32_t, little-endian)
 *   52     [13]   torpedo         (uint32_t, little-endian)
 *
 *
 *   While we are no longer using the array system, I think that keeping it within an array format will
 *   make it much easier to add values on the future - servo and dropper in particular.
 *   Nothing will need to change, just define the index and memcopy the bitch
 */

#define USB_WORD_SIZE              4

#define USB_IDX_MOTOR_0            0
#define USB_IDX_KILL_STATE         8
#define USB_IDX_POWER_OFF          9
#define USB_IDX_SERVO_1            10
#define USB_IDX_SERVO_2            11
#define USB_IDX_DROPPER            12
#define USB_IDX_TORPEDO            13

#define USB_OFFSET(idx)            ((idx) * USB_WORD_SIZE)
#define USB_PACKET_MIN_LEN         56   // 14 fields × 4 bytes



int8_t SystemState_UpdateFromUSB(SystemState *state, const uint8_t *buffer, uint8_t len){

	if (len < USB_PACKET_MIN_LEN){
		return -1;
	}

	//memcpy(dest, src, num_bytes)

	//all 32 bytes of motors
	memcpy(state->motorValues, buffer + USB_OFFSET(USB_IDX_MOTOR_0), sizeof(state->motorValues));

	//killstate
	memcpy(&state->killState, buffer + USB_OFFSET(USB_IDX_KILL_STATE), USB_WORD_SIZE);

	//poweroff
	memcpy(&state->powerOff, buffer + USB_OFFSET(USB_IDX_POWER_OFF), USB_WORD_SIZE);

	//rgb values
    memcpy(&state->servo1,   buffer + USB_OFFSET(USB_IDX_SERVO_1),   USB_WORD_SIZE);
    memcpy(&state->servo2, buffer + USB_OFFSET(USB_IDX_SERVO_2), USB_WORD_SIZE);
    memcpy(&state->dropper,  buffer + USB_OFFSET(USB_IDX_DROPPER),  USB_WORD_SIZE);
    memcpy(&state->torpedo,  buffer + USB_OFFSET(USB_IDX_TORPEDO),  USB_WORD_SIZE);

    return 0;
}


SystemState pendingState;
volatile uint8_t stateUpdatePending = 0;

void SystemState_ApplyPendingUpdate(SystemState *state)
{
    if (!stateUpdatePending) {
        return;
    }

    __disable_irq();

    memcpy(state->motorValues, pendingState.motorValues, sizeof(state->motorValues));
    state->killState = pendingState.killState;
    state->powerOff  = pendingState.powerOff;
    state->servo1    = pendingState.servo1;
    state->servo2    = pendingState.servo2;
    state->dropper   = pendingState.dropper;
    state->torpedo   = pendingState.torpedo;

    stateUpdatePending = 0;
    __enable_irq();
}
