/*
 * motor_control.c
 *
 *  Created on: Jun 4, 2026
 *      Author: Trevor Young
 */


#include "motor_control.h"
#include "system_state.h"
#include "main.h"


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;

#define MOTOR_NEUTRAL 35999
#define MOTOR_DEADBAND 750
#define NUM_MOTORS 8
#define MOTOR_RANGE_MAX 8500
#define ESC_ARM_TIME_MS 3000

//forward declearations, im too lazy to reorder everything
static void Motor_Apply_Commands(void);
static void Start_PWMS(void);
static void Set_Motors_Neutral(void);
static void Set_Motor_PWM(TIM_HandleTypeDef *htim, int32_t value);

//used to represent startup states
typedef enum
{
    MOTOR_STATE_ARMING,
    MOTOR_STATE_READY,
    MOTOR_STATE_KILLED
} MotorState;

static TIM_HandleTypeDef *motorTimers[NUM_MOTORS] =
{
    &htim1, //motor 0
    &htim2,
    &htim3,
    &htim4,
    &htim5,
    &htim15,
    &htim16,
    &htim17 //motor 7
};

static MotorState motorState;
static uint32_t armingStartTime;
static SystemState *systemState; //static pointer for globalState - pointer does not change, values do


void Motor_Init(SystemState *state){

	systemState = state;

	Start_PWMS();
	Motor_Start_Arming();
}

static void Set_Motors_Neutral(void){
	for(uint8_t i = 0; i < NUM_MOTORS; i++){
		__HAL_TIM_SET_COMPARE(motorTimers[i], TIM_CHANNEL_1, MOTOR_NEUTRAL);
	}
}

void Motor_Update(void)
{
    switch(motorState)
    {
        case MOTOR_STATE_ARMING:

            if((HAL_GetTick() - armingStartTime) >= ESC_ARM_TIME_MS)
            {
                motorState = MOTOR_STATE_READY;
            }

            break;

        case MOTOR_STATE_READY:

        	state.motorsArming = 0; //LED stops flashing

            Motor_Apply_Commands();
            break;

        case MOTOR_STATE_KILLED:

        	Set_Motors_Neutral();
            break;

        default:
            break;
    }
}



static void Set_Motor_PWM(TIM_HandleTypeDef *htim, int32_t value)
{
    if((value > MOTOR_RANGE_MAX) || (value < -MOTOR_RANGE_MAX)) //if motor values exceed motor range set to neutral
    {
        value = 0;
    }

    if(value == 0) //neutral
    {
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, MOTOR_NEUTRAL);
    }
    else if(value > 0) //forward
    {
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, MOTOR_NEUTRAL + MOTOR_DEADBAND + value);
    }
    else //reverse
    {
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, MOTOR_NEUTRAL - MOTOR_DEADBAND + value);
    }
}

static void Motor_Apply_Commands(void){
	for(uint8_t i = 0; i < NUM_MOTORS; i++){
		Set_Motor_PWM(motorTimers[i], systemState->motorValues[i]);
	}
}


static void Start_PWMS(void){
	for(uint8_t i = 0; i < NUM_MOTORS; i++){
		HAL_TIM_PWM_Start(motorTimers[i], TIM_CHANNEL_1);
	}
}

void Motor_Start_Arming(void){
	Set_Motors_Neutral();

	motorState = MOTOR_STATE_ARMING;
	armingStartTime = HAL_GetTick();  //use ticks instead of delays to allow other code to keep running

	state.motorsArming = 1; //variable to indicate motors are arming(used by LED)
}

void Motor_Kill(void)
{
    motorState = MOTOR_STATE_KILLED;
    Set_Motors_Neutral();
}

