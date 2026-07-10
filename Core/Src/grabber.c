/*
 * grabber.c
 *
 *  Created on: Jun 24, 2026
 *      Author: Trevor Young
 */


#include "grabber.h"
#include "system_state.h"
#include "main.h"



extern TIM_HandleTypeDef htim2;

#define GRABBER_NEUTRAL      35999   // 1500us - neutral
#define GRABBER_MIN          21599   // 900us  - min
#define GRABBER_MAX          45598   // 1900us - max
#define GRABBER_ARM_TIME_MS  3000

typedef enum {
    GRABBER_ARMING,
    GRABBER_READY,
    GRABBER_KILLED,
} GrabberState;

static GrabberState grabberState;
static uint32_t armingStartTime;
static SystemState *systemState;


static void Set_Grabber1_Neutral(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, GRABBER_NEUTRAL);
}

static void Set_Grabber2_Neutral(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, GRABBER_NEUTRAL);
}

static void Set_Grabbers_Neutral(void)
{
    Set_Grabber1_Neutral();
    Set_Grabber2_Neutral();
}

void Grabber_Start_Arming(void)
{
    Set_Grabbers_Neutral();
    grabberState    = GRABBER_ARMING;
    armingStartTime = HAL_GetTick();
}

void Grabber_Init(SystemState *state)
{
    systemState = state;
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
    Grabber_Start_Arming();
}

static void Set_Grabber_PWM(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t value)
{
    if (value < GRABBER_MIN || value > GRABBER_MAX)
    {
        __HAL_TIM_SET_COMPARE(htim, channel, GRABBER_NEUTRAL);
        return;
    }

    __HAL_TIM_SET_COMPARE(htim, channel, value);
}

void Grabber_Update(void)
{
    switch (grabberState)
    {
        case GRABBER_ARMING:
            if ((HAL_GetTick() - armingStartTime) >= GRABBER_ARM_TIME_MS)
            {
                grabberState = GRABBER_READY;
            }
            break;

        case GRABBER_READY:
        	Set_Grabber_PWM(&htim2, TIM_CHANNEL_4, systemState->servo1);
        	Set_Grabber_PWM(&htim2, TIM_CHANNEL_2, systemState->servo2);
            break;

        case GRABBER_KILLED:
            Set_Grabbers_Neutral();
            break;

        default:
            break;
    }
}


void Grabber_Kill(void)
{
    grabberState = GRABBER_KILLED;
    Set_Grabbers_Neutral();
}


