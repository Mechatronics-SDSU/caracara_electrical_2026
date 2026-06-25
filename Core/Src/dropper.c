/*
 * dropper.c
 *
 *  Created on: Jun 25, 2026
 *      Author: tyoung
 */

#include "dropper.h"
#include "system_state.h"
#include "main.h"

extern TIM_HandleTypeDef htim8;


#define DROPPER_NEUTRAL 35999  //is neutral value a closed state? if so we need to modify the update code
#define DROPPER_OPEN    26400 //1.1 ms
#define DROPPER_CLOSED  45600 //1.9 ms
#define DROPPER_ARM_TIME_MS  3000

typedef enum{
	DROPPER_READY,
	DROPPER_ARMING,
	DROPPER_KILLED,
}Dropper_State;

static DropperState dropperState;
static uint32_t armingStartTime;
static SystemState *systemState;

void Set_Dropper_Neutral(void){
	__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, DROPPER_NEUTRAL);
}

static void Dropper_Start_Arming(void)
{
    Set_Dropper_Neutral();
    dropperState = DROPPER_ARMING;
    armingStartTime = HAL_GetTick();
}

void Dropper_Init(SystemState *state){

	systemState = state;
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	Dropper_Start_Arming();
}

void Dropper_Update(void){
	switch(dropperState){
		case DROPPER_ARMING:
			if((HAL_GetTick() - armingStartTime) >= ESC_ARM_TIME_MS)
			{
				dropperState = DROPPER_READY;
	        }
            break;

		case DROPPER_READY:
			if (state->dropper){
				 __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, DROPPER_OPEN);
			}else{
				 __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, DROPPER_CLOSED);
			}
			break;

		case DROPPER_KILLED:
			Set_Dropper_Neutral();
			break;

		default:
			break;

	}
}

void Dropper_Kill(void)
{
    dropperState = DROPPER_KILLED;
    Dropper_Set_Neutral();
}


