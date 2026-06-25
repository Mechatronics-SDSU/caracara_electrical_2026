/*
 * grabber.c
 *
 *  Created on: Jun 24, 2026
 *      Author: tyoung
 */


#define GRABBER_OPEN    12000
#define GRABBER_CLOSED  48000
#define GRABBER_NEUTRAL 35999



void Grabber_Init(void){
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
}

void Set_Grabber_Neutral(void){
	__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 35999);
}
