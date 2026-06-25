///*
// * dumpForOldCode.c
// *
// *  Created on: Jun 5, 2026
// *      Author: tyoung
// */
//
//
// /* USER CODE END 2 */
//
//  /* Infinite loop */
//  /* USER CODE BEGIN WHILE */
//  while (1)
//  {
//
//
//
//	    uint32_t now = HAL_GetTick();
//
//	    switch (sensor_state)
//	    {
//	        case 0:
//	            // --- Start D1 conversion ---
//	            HAL_I2C_Master_Transmit(&hi2c1, MS5837_ADDR << 1, &MS5837_CONVERT_D1_8192, 1, 100);
//	            sensor_tick = now;
//	            sensor_state = 1;
//	            break;
//
//	        case 1:
//	            // --- Wait for D1 conversion (18 ms) ---
//	            if (now - sensor_tick >= 18)
//	            {
//	                // Read D1 (pressure)
//	                HAL_I2C_Master_Transmit(&hi2c1, MS5837_ADDR << 1, &MS5837_ADC_READ, 1, 100);
//	                HAL_I2C_Master_Receive(&hi2c1, MS5837_ADDR << 1, dataBuffer, 3, 100);
//	                D1_pressure = (dataBuffer[0] << 16) | (dataBuffer[1] << 8) | dataBuffer[2];
//
//	                // Start D2 conversion
//	                HAL_I2C_Master_Transmit(&hi2c1, MS5837_ADDR << 1, &MS5837_CONVERT_D2_8192, 1, 100);
//	                sensor_tick = now;
//	                sensor_state = 2;
//	            }
//	            break;
//
//	        case 2:
//	            // --- Wait for D2 conversion (18 ms) ---
//	            if (now - sensor_tick >= 18)
//	            {
//	                // Read D2 (temperature)
//	                HAL_I2C_Master_Transmit(&hi2c1, MS5837_ADDR << 1, &MS5837_ADC_READ, 1, 100);
//	                HAL_I2C_Master_Receive(&hi2c1, MS5837_ADDR << 1, dataBuffer, 3, 100);
//	                D2_temperature = (dataBuffer[0] << 16) | (dataBuffer[1] << 8) | dataBuffer[2];
//
//	                // Now do your compensation and depth math
//	                int32_t dT = D2_temperature - ((uint32_t)coefficients[5]) * 256L;
//	                int32_t TEMP = 2000L + (((int64_t)(dT) * coefficients[6]) / 8388608LL);
//	                int64_t OFF = ((int64_t)(coefficients[2]) * 131072L) + (((int64_t)(coefficients[4]) * dT) / 64L);
//	                int64_t SENS = (((int64_t)coefficients[1]) * 65536L) + (((int64_t)(coefficients[3]) * dT) / 128L);
//	                int32_t P = (((D1_pressure * SENS) / 2097152L) - OFF) / 32768L;
//
//	                float ATMOSPHERIC_PRESSURE = 101300.0f;
//	                uint32_t adjusted_pressure = P - ATMOSPHERIC_PRESSURE;
//	                float depth = adjusted_pressure / (1029.0f * 9.80665f);
//
//	      	      uint8_t header = 0xAA; // header at the start of each packet
//	      	      //float depth = 654321.0f;
//
//	      	      uint8_t packet[11];
//	      	      packet[0] = header;
//	      	      packet[1] = greenPressed ; //? '1' : '0';
//	      	      packet[2] = bluePressed ; // ? '1' : '0';
//	      	      packet[3] = extKillState; // ? '1' : '0'; // The state of the external magnet kill. 1 = Killed 0 = Not killed
//	      	      packet[4] = intKillState; //? '1' : '0'; // The state of the usb kill. 1 = microcontroller thinks the computer has transmitted a kill and 0 for not killed
//	      	      memcpy(&packet[5], &depth, 4);  // Little-endian format
//	      	      packet[9] = '\r';
//	      	      packet[10] = '\n';
//
//	      	      CDC_Transmit_FS(packet, sizeof(packet));  // Send packet over USB
//	      	      greenPressed = 0;
//	      	      bluePressed = 0;
//
//	      	      //HAL_Delay(1); // Delay so that there isn't an overwhelming amount of data to receive
//
//	                // Print tick + depth
//	                char float_usb_tx_buffer[64];
//	                sprintf((char*)float_usb_tx_buffer, "Tick: %lu ms | Depth: %.7f meters\r\n\n", now, depth);
//	                CDC_Transmit_FS((uint8_t *)float_usb_tx_buffer, strlen((char*)float_usb_tx_buffer));
//
//	                // Start next D1 conversion right away
//	                HAL_I2C_Master_Transmit(&hi2c1, MS5837_ADDR << 1, &MS5837_CONVERT_D1_8192, 1, 100);
//	                sensor_tick = now;
//	                sensor_state = 1;
//	            }
//	            break;
//	    }
//
//
//
//
//
//
//
//		// Read data
//		ret = HAL_I2C_Master_Transmit(&hi2c1, MS5837_ADDR << 1, &MS5837_CONVERT_D1_8192, 1, 5000);
//		HAL_Delay(18); // Max conversion time per datasheet
//
//		ret = HAL_I2C_Master_Transmit(&hi2c1, MS5837_ADDR << 1, &MS5837_ADC_READ, 1, 5000);
//		uint8_t dataBuffer[3]; // Buffer to hold the 3 bytes received
//		HAL_I2C_Master_Receive(&hi2c1, MS5837_ADDR << 1, dataBuffer, 3, HAL_MAX_DELAY);
//
//		uint32_t D1_pressure = (dataBuffer[0] << 16) | (dataBuffer[1] << 8) | dataBuffer[2];
//		char d1_usb_tx_buffer[64]; // Ensure buffer is large enough
//		sprintf((char*)d1_usb_tx_buffer, "D1 Pressure: %lu\r\n", D1_pressure);
//		CDC_Transmit_FS((uint8_t *)d1_usb_tx_buffer, strlen((char*)d1_usb_tx_buffer));
//
//		ret = HAL_I2C_Master_Transmit(&hi2c1, MS5837_ADDR << 1, &MS5837_CONVERT_D2_8192, 1, 5000);
//		HAL_Delay(18); // Max conversion time per datasheet
//
//		ret = HAL_I2C_Master_Transmit(&hi2c1, MS5837_ADDR << 1, &MS5837_ADC_READ, 1, 5000);
//		HAL_I2C_Master_Receive(&hi2c1, MS5837_ADDR << 1, dataBuffer, 3, HAL_MAX_DELAY);
//
//		uint32_t D2_temperature = (dataBuffer[0] << 16) | (dataBuffer[1] << 8) | dataBuffer[2];
//		char d2_usb_tx_buffer[64]; // Ensure buffer is large enough
//		sprintf((char*)d2_usb_tx_buffer, "D2 Pressure: %lu\r\n", D2_temperature);
//		CDC_Transmit_FS((uint8_t *)d2_usb_tx_buffer, strlen((char*)d2_usb_tx_buffer));
//
//		// Calculate
//		// Perform Compensation on received temperature and pressure values
//		int32_t dT = 0;
//		int64_t SENS = 0;
//		int64_t OFF = 0;
//		int32_t SENSi = 0;
//		int32_t OFFi = 0;
//		int32_t Ti = 0;
//		int64_t OFF2 = 0;
//		int64_t SENS2 = 0;
//		int32_t P = 0;
//		int32_t P2 = 0;
//		int32_t TEMP = 0;
//		int32_t TEMP2 = 0;
//
//		char dT_buffer[64];
//		char SENS_buffer[64];
//		char OFF_buffer[64];
//		char SENSi_buffer[64];
//		char OFFi_buffer[64];
//		char Ti_buffer[64];
//		char OFF2_buffer[64];
//		char SENS2_buffer[64];
//		char P_buffer[64];
//		char TEMP_buffer[64];
//
//		// Terms called
//		dT = D2_temperature - ((uint32_t)coefficients[5]) * 256L;
//
//		// Temp conversion
//		TEMP = 2000L + (((int64_t)(dT) * coefficients[6]) / 8388608LL);
//
//		OFF = ((int64_t)(coefficients[2]) * 131072L) + (((int64_t)(coefficients[4]) * dT) / 64L);
//		SENS = (((int64_t)coefficients[1]) * 65536L) + (((int64_t)(coefficients[3]) * dT) / 128L);
//		P = (((D1_pressure * SENS) / 2097152L) - OFF) / 32768L; //good
//
//
//
//		// Print dT (int32_t)
//		sprintf(dT_buffer, "dT: %" PRId32 "\r\n", dT);
//		CDC_Transmit_FS((uint8_t *)dT_buffer, strlen(dT_buffer));
//		HAL_Delay(500);
//
//		// Print SENS (int64_t)
//		//sprintf(SENS_buffer, "SENS: %" PRIx64 "\r\n", SENS);
//		sprintf(SENS_buffer, "SENS: %lld\r\n", (long long)SENS);
//		CDC_Transmit_FS((uint8_t *)SENS_buffer, strlen(SENS_buffer));
//		HAL_Delay(500);
//
//		// Print OFF (int64_t)
//		sprintf(OFF_buffer, "OFF: %" PRId64 "\r\n", OFF);
//		CDC_Transmit_FS((uint8_t *)OFF_buffer, strlen(OFF_buffer));
//		HAL_Delay(500);
//
//		// Second order compensation
//		// Low temp
//		if ((TEMP / 100) < 20)
//		{
//			Ti = (11 * (int64_t)(dT) * (int64_t)(dT)) / (34359738368LL);
//			OFFi = (31 * (TEMP - 2000) * (TEMP - 2000)) / 8;
//			SENSi = (63 * (TEMP - 2000) * (TEMP - 2000)) / 32;
//		}
//
//		OFF2 = OFF - OFFi;           // Calculate pressure and temp second order
//		SENS2 = SENS - SENSi;
//
//		TEMP2 = (TEMP - Ti) / 100;
//		sprintf(TEMP_buffer, "Temp2: %" PRId32 "\r\n", TEMP2);
//		CDC_Transmit_FS((uint8_t *)TEMP_buffer, strlen(TEMP_buffer));
//		HAL_Delay(500);
//
//		//P = (((D1_pressure * SENS2) / 2097152L - OFF2) / 32768L) / 100; changed
//		P2 = ((D1_pressure * SENS2) / 2097152L - OFF2) / 32768L / 100;
//		sprintf(P_buffer, "P2: %" PRId32 "\r\n", P2);
//		CDC_Transmit_FS((uint8_t *)P_buffer, strlen(P_buffer));
//		HAL_Delay(500);
//
//		// TEMP and P looking good
//
//		// suspicion is that the order is off - do it following the data sheet
//
//		//SECOND ORDER
//		float ATMOSPHERIC_PRESSURE = 101300.0f; // Pa
//
//		uint32_t adjusted_pressure = P - ATMOSPHERIC_PRESSURE; // Adjust pressure;
//		//uint32_t adjusted_pressure = P; // Adjust pressure;
//		float depth = adjusted_pressure / (1029 * 9.80665); // Depth in meters
//		uint32_t temperature = TEMP;
//
//		char long_usb_tx_buffer[64]; // Ensure buffer is large enough
//		sprintf((char*)long_usb_tx_buffer, "Adjusted Pressure: %lu\r\n", adjusted_pressure);
//		CDC_Transmit_FS((uint8_t *)long_usb_tx_buffer, strlen((char*)long_usb_tx_buffer));
//		HAL_Delay(500);
//
//		char temp_long_usb_tx_buffer[64]; // Ensure buffer is large enough
//		sprintf((char*)temp_long_usb_tx_buffer, "Compensated Temp: %lu\r\n", temperature);
//		CDC_Transmit_FS((uint8_t *)temp_long_usb_tx_buffer, strlen((char*)temp_long_usb_tx_buffer));
//		HAL_Delay(500);
//
//
//		uint32_t tick = HAL_GetTick();
//		char float_usb_tx_buffer[64]; // Ensure buffer is large enough
//		sprintf((char*)float_usb_tx_buffer, "Tick: %lu ms | Depth: %.7f meters\r\n\n", tick, depth);
//		sprintf((char*)float_usb_tx_buffer, "Depth: %.7f meters\r\n\n", depth);
//		CDC_Transmit_FS((uint8_t *)float_usb_tx_buffer, strlen((char*)float_usb_tx_buffer));
//		HAL_Delay(5);
//
//
//
//
//
//
//
//
//
//
//	  GPIO_PinState usbPresent = HAL_GPIO_ReadPin(GPIOC, VBUS_DETECT_Pin);
//
//	  if (usbPresent){
//		  HAL_GPIO_WritePin(GPIOC, LED_RED_Pin, GPIO_PIN_RESET);
//		  HAL_GPIO_WritePin(GPIOA, D__PU_Pin, GPIO_PIN_RESET);
//	  }
//	  else {
//		  HAL_GPIO_WritePin(GPIOA, D__PU_Pin, GPIO_PIN_RESET);
//		  HAL_GPIO_WritePin(GPIOC, LED_RED_Pin, GPIO_PIN_SET);
//	  }
//
//	    CDC_Transmit_FS((uint8_t *)usb_tx_buffer, sizeof(usb_tx_buffer) - 1);
//
//	  HAL_GPIO_WritePin(GPIOC, LED_RED_Pin, GPIO_PIN_RESET);
//	  HAL_Delay(400);
//	  HAL_GPIO_WritePin(GPIOC, LED_RED_Pin, GPIO_PIN_SET);
//	  HAL_Delay(400);
//
//
//	  if (okayToRunMotorsYet == 1){
//	  Set_TIM_Compare(&htim1, usbBuffer[0]);
//	  Set_TIM_Compare(&htim2, usbBuffer[1]);
//	  Set_TIM_Compare(&htim3, usbBuffer[2]);
//	  Set_TIM_Compare(&htim4, usbBuffer[3]);
//	  Set_TIM_Compare(&htim5, usbBuffer[4]);
//	  Set_TIM_Compare(&htim15, usbBuffer[5]);
//	  Set_TIM_Compare(&htim16, usbBuffer[6]);
//	  Set_TIM_Compare(&htim17, usbBuffer[7]);
//	  }
//
//	  if (usbBuffer[8] !=0){ // if the kill value isn't zero then set the internal kill pin high
//		  HAL_GPIO_WritePin(GPIOC, INT_KILL_Pin, GPIO_PIN_SET);
//		  intKillState = 1;
//		  prevIntKillState = 1;
//	  }
//
//	  else {
//		  HAL_GPIO_WritePin(GPIOC, INT_KILL_Pin, GPIO_PIN_RESET);
//		  intKillState = 0;
//	  }
//
//	  if ((usbBuffer[9] == 1) || (powerDown == 1)){ // if the button or usb packet triggers power down then set power off pin high
//		  HAL_GPIO_WritePin(GPIOC, POWER_OFF_Pin, GPIO_PIN_SET);
//	  }
//
//	  extKillState = HAL_GPIO_ReadPin(EXT_KILL_GPIO_Port, EXT_KILL_Pin);
//
//	  if  ((extKillState == 1) && (prevExtKillState == 0)){
//		  prevExtKillState = 1;
//
//		  // if the ext kill state is high then set the motor values to zero
//		  //so that hopefully the motors don't immediately spin after kill magnet is replaced.
//
//		  for (uint8_t i = 0; i < 8; i++) {
//		      usbBuffer[i] = 0;
//		  }
//		  // fixme send usb data telling computer the sub is supposed to be killed so that the motors don't continue running after un-killing
//	  }
//
//	  if (((prevExtKillState == 1) && (extKillState == 0) && (intKillState == 0)) || ((intKillState == 0) && (prevIntKillState == 1) && (extKillState == 0))){
//		  	  // if we were killed before and now we aren't and neither of the kills are triggered anymore then calibrate the ESC's
//		  prevExtKillState = 0;
//		  prevIntKillState = 0;
//		  okayToRunMotorsYet = 0;
//
//		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 35999); // set all motors to 1500uS so they can be calibrated after being re-powered
//		  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 35999);
//		  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 35999);
//		  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 35999);
//		  __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 35999);
//		  __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_1, 35999);
//		  __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 35999);
//		  __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, 35999);
//
//	  	  	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
//	    	  __HAL_TIM_SET_COUNTER(&htim7, 0);
//	  	    HAL_TIM_Base_Start_IT(&htim7); // start 3 second timer to calibrate ESC's
//	  }
//
///*
//	    char ascii_str[6];  // Buffer to hold the ASCII string (enough to store 4 digits + null terminator)
//
//	    sprintf(ascii_str, "%d", tempData);
//
//	    CDC_Transmit_FS((uint8_t *)ascii_str, sizeof(ascii_str) - 1);
//
//        CDC_Transmit_FS(usb_tx_buffer, sizeof(usb_tx_buffer) - 1);
//
//	    sprintf(ascii_str, "%d", rx_data);
//
//	    CDC_Transmit_FS((uint8_t *)ascii_str, sizeof(ascii_str) - 1);
//
//	    */
//
//
//	      GPIO_PinState buttonState = HAL_GPIO_ReadPin(GPIOC, BUTTON_IN_Pin); // read the button status
//
//	      if (buttonState == GPIO_PIN_RESET && (buttonPushedAlready == 0)) // if the button gets pulled low (pushed) and it wasn't pulled low on the previous cycle
//	  {
//	    	  wentLowTime = HAL_GetTick();
//	    	  lockColor = 1; // lock the current led color so that we can blink it repeatedly
//	    	  buttonPushedAlready = 1; // this lets us know the button it already pushed so it doesn't get stuck if the button is held down
//
//	    	  if (color == 'G'){ // if the red led is on, the 'G' is confusing but this is what the color variable gets set to when the red led is on
//
//
//	    		  /*we will use red to turn the sub off.
//	    		   * The red led will flash slowly and the user will have to push it a
//	    		   * second time in order to avoid accidentally powering the sub off
//	    		   */
//
//	    		  __HAL_TIM_SET_AUTORELOAD(&htim6, 7500); // give a longer period for the red led
//	    		  checkRedMode = 1; // enter checkRedMode (we check for the second button push)
//	    	  }
//	    	  else {
//
//	    		  if (color == 'B'){ // happens when the actual led is green
//
//		    		  //CDC_Transmit_FS((uint8_t *)"Green\r\n", 7);
//		    		  greenPressed = 1;
//		    		  // fix me transmit something over USB so software can know we want to enter autonomous mode or whatever
//	    		  }
//	    		  else if (color == 'R'){ // happens when the actual led is Blue
//		    		  //CDC_Transmit_FS((uint8_t *)"Blue\r\n", 6);
//	    			  bluePressed = 1;
//		    		  // fix me transmit something over USB so software can know we want to enter autonomous mode or whatever
//	    		  }
//	    	  __HAL_TIM_SET_AUTORELOAD(&htim6, 2000); // for non red colors we blink the leds faster
//	    	  }
//	    	  __HAL_TIM_SET_COUNTER(&htim6, __HAL_TIM_GET_AUTORELOAD(&htim6) - 1); // force the previous timer to overflow, otherwise the led might stay on for a bit before the blink starts
//	  }
//
//	      else if ((buttonPushedAlready == 1) && (buttonState == GPIO_PIN_SET) && ((HAL_GetTick() - wentLowTime) > 200)){
//	    	  wentHighTime = HAL_GetTick();
//	    	  buttonReleased = 1; // we know the button was released
//	      }
//
//	      if ((checkRedMode == 1) && (buttonReleased == 1) && (buttonState == GPIO_PIN_RESET) && ((HAL_GetTick() - wentHighTime) > 200)){
//	    	  powerDown = 1; // power down if the red led is flashing and gets pressed a second time
//    		  //CDC_Transmit_FS((uint8_t *)"Power Off\r\n", 11);
//	    	  // note that switch bounce is an issue with a standard push button. Hopefully the piezo button won't have any issues
//	      }
//
//
//
//
//
//
//
//
//
//	      if ((rgbRed != usbBuffer[10]) || (rgbGreen != usbBuffer[11]) || (rgbBlue != usbBuffer[12])){ // If the received color data changes
//
//	      rgbRed = usbBuffer[10];
//	      rgbGreen = usbBuffer[11];
//	      rgbBlue = usbBuffer[12];
//
//	      char uartBuffer[20]; // Enough space for "255 000 010\r\n"
//
//	      // Format into ASCII string with leading zeros
//	      sprintf(uartBuffer, "%03lu %03lu %03lu\n",
//	              (unsigned long)rgbRed,
//	              (unsigned long)rgbGreen,
//	              (unsigned long)rgbBlue);
//
//	      // Transmit via UART
//	      HAL_UART_Transmit(&huart2, (uint8_t*)uartBuffer, strlen(uartBuffer), HAL_MAX_DELAY);
//	      }
//
//    /* USER CODE END WHILE */
//
//    /* USER CODE BEGIN 3 */
//  }
//  /* USER CODE END 3 */
//}
