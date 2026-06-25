/*
 * button_state.c
 *
 *  Created on: Jun 17, 2026
 *      Author: tyoung
 */

#include "system_state.h"
#include "stm32g4xx_hal.h"
#include "main.h"
#include "button_state.h"

/*Weird LED color rules - should be replaced with an enum
 * 'R' = Blue LED is on
 * 'G' = Red LED is on
 * 'B' = Green LED is on
 */

extern TIM_HandleTypeDef htim6;

static uint8_t color = 'R';
static uint8_t lockColor = 0;
static uint8_t buttonPushedAlready = 0;
static uint8_t buttonReleased = 0;
static uint8_t buttonRepressed = 0;
static uint8_t checkRedMode = 0;
static uint32_t wentLowTime = 0;
static uint32_t wentHighTime = 0;
static uint8_t greenPressed;
static uint8_t bluePressed;
static uint8_t powerDown;



void Button_Update(SystemState *state){


	GPIO_PinState buttonState = HAL_GPIO_ReadPin(GPIOC, BUTTON_IN_Pin); // read the button status


	if ((buttonState == GPIO_PIN_RESET) && (buttonPushedAlready == 0)) // if the button gets pulled low (pushed) and it wasn't pulled low on the previous cycle
	  {
	   	  wentLowTime = HAL_GetTick();
	   	  lockColor = 1; // lock the current led color so that we can blink it repeatedly
    	  buttonPushedAlready = 1; // this lets us know the button it already pushed so it doesn't get stuck if the button is held down
    	  buttonReleased = 0; //reset release flag so a new press starts clean

		  if (color == 'G'){ // if the red led is on, the 'G' is confusing but this is what the color variable gets set to when the red led is on


		     /*we will use red to turn the sub off.
		      * The red led will flash slowly and the user will have to push it a
		      * second time in order to avoid accidentally powering the sub off
		      */

			  __HAL_TIM_SET_AUTORELOAD(&htim6, 7500); // give a longer period for the red led
		      checkRedMode = 1; // enter checkRedMode (we check for the second button push)
		   }
		  else{
			   __HAL_TIM_SET_AUTORELOAD(&htim6, 2000); // for non red colors we blink the leds faster
		   }
		    if(color == 'B'){
		    	//CDC_Transmit_FS((uint8_t *)"Green\r\n", 7);
		    	greenPressed = 1;
		    	// fix me transmit something over USB so software can know we want to enter autonomous mode or whatever
		    }
		    else if (color == 'R'){ // happens when the actual led is Blue
			    //CDC_Transmit_FS((uint8_t *)"Blue\r\n", 6);
		    	bluePressed = 1;
			    // fix me transmit something over USB so software can know we want to enter autonomous mode or whatever
		    }
		   __HAL_TIM_SET_COUNTER(&htim6, __HAL_TIM_GET_AUTORELOAD(&htim6) - 1); // force the previous timer to overflow, otherwise the led might stay on for a bit before the blink starts
	  }
		if ((buttonPushedAlready == 1) && (buttonState == GPIO_PIN_SET) && ((HAL_GetTick() - wentLowTime) > 200)){
		    wentHighTime = HAL_GetTick();
		    buttonReleased = 1; // we know the button was released
		    buttonPushedAlready = 0; //clear pushed flag so next press is detected
		}

		if ((checkRedMode == 1) && (buttonReleased == 1) && (buttonState == GPIO_PIN_RESET) && ((HAL_GetTick() - wentHighTime) > 200)){
		    powerDown = 1; // power down if the red led is flashing and gets pressed a second time
		    state->powerDownButton = 1;
	        //CDC_Transmit_FS((uint8_t *)"Power Off\r\n", 11);
		    // note that switch bounce is an issue with a standard push button. Hopefully the piezo button won't have any issues
		}
}


void Button_TimerCallback(void)
{
	if (lockColor == 0){ // if the button hasn't been pushed then go ahead and change to the next color

		HAL_GPIO_WritePin(GPIOB, RED_Pin|GREEN_Pin|BLUE_Pin, GPIO_PIN_RESET); // turn off the previous led

	    if (color == 'R'){ // 'R' stands for red
	    	  HAL_GPIO_WritePin(GPIOB, RED_Pin, GPIO_PIN_SET); // turn on red led
	    	  color = 'G'; // next color is green
	    }

	    else if (color == 'G'){ // 'G' stands for green
	    	  HAL_GPIO_WritePin(GPIOB, GREEN_Pin, GPIO_PIN_SET); // turn on green led
	    	  color = 'B'; // next color is blue
	    }

	    else if (color == 'B'){ // 'B' stands for blue
	    	  HAL_GPIO_WritePin(GPIOB, BLUE_Pin, GPIO_PIN_SET); // turn on blue led
	    	  color = 'R'; // next color is blue
	    }
	}

	else{
	    lockColor ++;

	    if (color == 'G'){
	    HAL_GPIO_TogglePin(GPIOB,  RED_Pin);
	    }

	    else if (color == 'B'){
	    HAL_GPIO_TogglePin(GPIOB,  GREEN_Pin);
	    }

	    else if (color == 'R'){
	    HAL_GPIO_TogglePin(GPIOB,  BLUE_Pin);
	    }

	    if (lockColor == 20){ // toggle whatever led was pressed this many times
	    	lockColor = 0; // reset lock color so that we can change to the next color
	    	checkRedMode = 0; // exit checkRedMode
	    	buttonPushedAlready = 0; // reset this so that we can accept a new button input
	    	buttonReleased = 0;
	    	__HAL_TIM_SET_AUTORELOAD(&htim6, 65535); // set the timer period to something slower
	    	__HAL_TIM_SET_COUNTER(&htim6, __HAL_TIM_GET_AUTORELOAD(&htim6) - 1); // force the timer to overflow so we don't have to wait for the previous led blink to finish

	    }
	 }
}
