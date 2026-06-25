/*
 * usb_manager.c
 *
 *  Created on: Jun 17, 2026
 *      Author: Trevor Young
 */

#include "system_state.h"
#include "usb_manager.h"
#include "main.h"


void USB_Manager_Update(SystemState *state){
	GPIO_PinState usbPresent = HAL_GPIO_ReadPin(GPIOC, VBUS_DETECT_Pin);

	if (usbPresent){
		  HAL_GPIO_WritePin(GPIOA, D__PU_Pin, GPIO_PIN_SET);
		  state->usbConnected = 1;
	  }
	  else {
		  HAL_GPIO_WritePin(GPIOA, D__PU_Pin, GPIO_PIN_RESET);
		  state->usbConnected = 0;
	  }

}
