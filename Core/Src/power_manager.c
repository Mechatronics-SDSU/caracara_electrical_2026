/*
 * power_manager.c
 *
 *  Created on: Jun 6, 2026
 *      Author: Trevor Young
 */

#include "power_manager.h"
#include "system_state.h"
#include "main.h"
#include "bitmask.h"

//the power off pin is connected to some external board, and i am unsure of what that board does
//if issues arise with using that board, uncomment the below code to simulate a soft shutdown

//#include "motor_control.h"
#define POWER_DOWN_BIT_IDX 9

void Power_Down(SystemState *state){
	if (Bitmask_GetBit(state->bitmask, POWER_DOWN_BIT_IDX)){
		if ((state->powerOff) || (state->powerDownButton)){
		    //HAL_GPIO_WritePin(GPIOC, LED_RED_Pin|LED_GREEN_Pin|LED_BLUE_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, POWER_OFF_Pin, GPIO_PIN_SET);

		}
	}
}

