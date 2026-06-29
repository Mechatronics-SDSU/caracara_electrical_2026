/*
 * depth_sensor.h
 *
 *  Created on: Jun 27, 2026
 *      Author: tyoung
 */

#ifndef INC_DEPTH_SENSOR_H_
#define INC_DEPTH_SENSOR_H_


#include "stm32g4xx_hal.h"
#include "system_state.h"

//I2C address
#define MS5837_ADDR             0x76

//Available sensor commands
#define MS5837_RESET            0x1E
#define MS5837_PROM_READ        0xA0
#define MS5837_CONVERT_D1_8192  0x4A
#define MS5837_CONVERT_D2_8192  0x5A
#define MS5837_ADC_READ         0x00


/**
 * @brief  Initialise the MS5837 sensor: reset, read and validate PROM
 *         calibration coefficients.
 * @param  hi2c   Pointer to the I2C handle the sensor is wired to.
 * @param  state  Pointer to the global SystemState struct.
 * @retval HAL_OK on success, HAL_ERROR if reset or CRC fails.
 */
HAL_StatusTypeDef Depth_Sensor_Init(I2C_HandleTypeDef *hi2c, SystemState *state);

/**
 * @brief  Non-blocking update, call once per main-loop iteration.
 *         Drives the D1/D2 conversion state machine, computes depth,
 *         and transmits the 10-byte USB packet to Orin when ready.
 */
void Depth_Sensor_Update(void);



#endif /* INC_DEPTH_SENSOR_H_ */
