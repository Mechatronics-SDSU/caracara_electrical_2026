/*
 * rgb_led.c
 *
 *  Created on: Jun 18, 2026
 *      Author: tyoung
 */


#include "rgb_led.h"
#include "system_state.h"
#include "main.h"

#include <stdint.h>


void RGB_LED_Update(const SystemState *state){
	static uint32_t lastFlashTick = 0;
	static uint8_t flashState = 0;

	if (state->intKillState || state->extKillState){ //flash red if kill state active
		if ((HAL_GetTick() - lastFlashTick) >= 500)
		        {
		            lastFlashTick = HAL_GetTick();
		            flashState = !flashState;

		            HAL_GPIO_WritePin(GPIOC, LED_RED_Pin|LED_GREEN_Pin|LED_BLUE_Pin, GPIO_PIN_SET);

		            if (flashState)
		            {
		                HAL_GPIO_WritePin(GPIOC, LED_RED_Pin, GPIO_PIN_RESET);
		            }
		        }
	}
	else if (state->motorsArming){
		if ((HAL_GetTick() - lastFlashTick) >= 500)
		    {
		        lastFlashTick = HAL_GetTick();
		        flashState = !flashState;

		        HAL_GPIO_WritePin(GPIOC, LED_RED_Pin|LED_GREEN_Pin|LED_BLUE_Pin, GPIO_PIN_SET);

		        if (flashState)
		        {
		            HAL_GPIO_WritePin(GPIOC, LED_BLUE_Pin, GPIO_PIN_RESET);
		        }
		 }
	}
	else if(state->usbConnected){ //solid green if USB connected
		HAL_GPIO_WritePin(GPIOC, LED_BLUE_Pin|LED_GREEN_Pin|LED_RED_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, LED_GREEN_Pin, GPIO_PIN_RESET);
	}
	else{//solid red if usb diconnected
		HAL_GPIO_WritePin(GPIOC, LED_BLUE_Pin|LED_GREEN_Pin|LED_RED_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, LED_RED_Pin, GPIO_PIN_RESET);
	}
}

