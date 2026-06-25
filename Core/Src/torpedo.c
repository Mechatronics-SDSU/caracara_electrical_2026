/*
 * torpedo.c
 *
 *  Created on: Jun 24, 2026
 *      Author: Trevor Young
 */


#include "torpedo.h"
#include "system_state.h"
#include "main.h"

extern TIM_HandleTypeDef htim20;

#define TORPEDO_NEUTRAL  35999  // 1.5ms center
#define TORPEDO_ARMED    26400  // 1.1ms — servo holds torpedo
#define TORPEDO_FIRED    45600  // 1.9ms — servo releases torpedo
#define TORPEDO_ARM_TIME_MS  3000




  void Torpedo_Init(void){
	  HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_1);
  }


  void Torpedo_Neutral(void){
	  __HAL_TIM_SET_COMPARE(&htim20, TIM_CHANNEL_1, 35999);// torpedo pwm

  }
