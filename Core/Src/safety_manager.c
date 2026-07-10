/*
 * kill.c
 *
 *  Created on: Jun 6, 2026
 *      Author: Trevor Young
 */

#include "safety_manager.h"
#include "system_state.h"
#include "motor_control.h"
#include "dropper.h"
#include "torpedo.h"
#include "grabber.h"
#include "main.h"

#include <stdint.h>

//internal kill represents kill command come from usbpacket
//external kill represents kill command come from button
//previous values used to detect edge cases
static uint8_t prevIntKillState = 0;
static uint8_t prevExtKillState = 0;

void Safety_Manager_Update(SystemState *state){

	state->extKillState = HAL_GPIO_ReadPin(EXT_KILL_GPIO_Port, EXT_KILL_Pin);

	//if usb kill value is one we set internal kill pin high
	if (state->killState){
		HAL_GPIO_WritePin(GPIOC, INT_KILL_Pin, GPIO_PIN_SET);
		state->intKillState = 1;
		prevIntKillState = 1;

		Motor_Kill();
		Dropper_Kill();

	}
	else {
		HAL_GPIO_WritePin(GPIOC, INT_KILL_Pin, GPIO_PIN_RESET);
		state->intKillState = 0;
	}

	if ((state->extKillState) && (prevExtKillState == 0)){
		prevExtKillState = 1;

		Motor_Kill();
		Dropper_Kill();

	}

	//if we were killed, but are now being restart, then set previous Kill states to zero and restart the motors
	if (((prevExtKillState == 1) && (state->extKillState == 0) && (state->intKillState == 0)) || ((prevIntKillState == 1) && (state->extKillState == 0) && (state->intKillState == 0))){
			  prevExtKillState = 0;
			  prevIntKillState = 0;

			  Motor_Start_Arming();
			  Dropper_Start_Arming();

		  }
}



/*future considerations - if ext/intKillState is not used anywhere else in the code, isolate it to this
 * singular file. Additionally, some these conditionals are horribly lengthy, can combine prevKillStates and
 * current kill states into two bools instead of four. Lastly state->killState = state->intKillState, a
 * redundancy that needs to be fixed.
 */

