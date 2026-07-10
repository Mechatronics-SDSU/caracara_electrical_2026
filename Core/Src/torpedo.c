/*
 * torpedo.c
 *
 *  Created on: Jun 24, 2026
 *      Author: Trevor Young
 */


/*
 * torpedo.c
 *
 *  Created on: Jun 25, 2026
 *      Author: tyoung
 */

#include "torpedo.h"
#include "system_state.h"
#include "main.h"

extern TIM_HandleTypeDef htim20;

/*
 * Neutral(1500μs) = 1500 * 65536 / 2730 ≈ 35999
 * Min(900 microseconds) = 900  * 65536 / 2730 ≈ 21599
 * Max(1900 microseconds) = 1900 * 65536 / 2730 = 45599
 */

#define TORPEDO_NEUTRAL      35999  // neutral is closed state
#define TORPEDO_OPEN         45599  // 1.9ms - fires torpedo
#define TORPEDO_ARM_TIME_MS  3000

typedef enum{
    TORPEDO_ARMING,
    TORPEDO_READY,
    TORPEDO_KILLED,
} TorpedoState;

static TorpedoState torpedoState;
static uint32_t armingStartTime;
static SystemState *systemState;

static void Set_Torpedo_Neutral(void){
    __HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_1, TORPEDO_NEUTRAL);
}

void Torpedo_Start_Arming(void){
    Set_Torpedo_Neutral();
    torpedoState = TORPEDO_ARMING;
    armingStartTime = HAL_GetTick();
}

void Torpedo_Init(SystemState *state){
    systemState = state;
    HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_1);
    Torpedo_Start_Arming();
}

void Torpedo_Update(void){
    switch(torpedoState){
        case TORPEDO_ARMING:
            if((HAL_GetTick() - armingStartTime) >= TORPEDO_ARM_TIME_MS)
            {
                torpedoState = TORPEDO_READY;
            }
            break;

        case TORPEDO_READY:
            if(systemState->torpedo){
                __HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_1, TORPEDO_OPEN);
                /*
            	char buf[64];
            	sprintf(buf, "TORPEDO_READY reached, torpedo=%lu\r\n", systemState->torpedo);
            	CDC_Transmit_FS((uint8_t*)buf, strlen(buf));
            	*/
            }
            else
                Set_Torpedo_Neutral();
            break;

        case TORPEDO_KILLED:
            Set_Torpedo_Neutral();
            break;

        default:
            break;
    }
}

void Torpedo_Kill(void){
    torpedoState = TORPEDO_KILLED;
    Set_Torpedo_Neutral();
}
