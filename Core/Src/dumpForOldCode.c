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

