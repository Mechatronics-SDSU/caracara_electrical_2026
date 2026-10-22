/*
 * dropper.c
 *
 *  Created on: Jun 25, 2026
 *      Author: tyoung
 */

#include "dropper.h"
#include "system_state.h"
#include "main.h"
#include "bitmask.h"

extern TIM_HandleTypeDef htim8;

/*
 * Neutral(1500μs) = 1500 * 65536 / 2730 ≈ 35999
 * Min(900 microseconds) = 900  * 65536 / 2730 ≈ 21599
 * Max(1900 microseconds) = 1900 * 65536 / 2730 = 45599
 */

#define DROPPER_NEUTRAL 	 1000 	//is neutral value is closed state
#define DROPPER_MIN     	 0    	//min
#define DROPPER_MAX    	     2000 	//1.1 ms
#define DROPPER_ARM_TIME_MS  3000
#define DROPPER_BITMASK_IDX  12

typedef enum{
	DROPPER_READY,
	DROPPER_ARMING,
	DROPPER_KILLED,
}DropperState;

static DropperState dropperState;
static uint32_t armingStartTime;
static SystemState *systemState;

static void Set_Dropper_Neutral(void){
	__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, DROPPER_NEUTRAL);
}

void Dropper_Start_Arming(void)
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
			if((HAL_GetTick() - armingStartTime) >= DROPPER_ARM_TIME_MS)
			{
				dropperState = DROPPER_READY;
	        }
            break;

		case DROPPER_READY:
			if (!(Bitmask_GetBit(systemState->bitmask, DROPPER_BITMASK_IDX))){
					break;
			}
			else if((systemState->dropper < 0)) //if motor values exceed motor range set to neutral
            {
				Set_Dropper_Neutral();
		     }
             else if(systemState->dropper == 1000) //neutral
             {
              __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1,DROPPER_NEUTRAL);
		     }
		     else //reverse
		     {
			 __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, systemState->dropper);
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
    Set_Dropper_Neutral();
}


