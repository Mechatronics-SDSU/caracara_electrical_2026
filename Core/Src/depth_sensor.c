/*
 * depth_sensor.c
 *
 *  Created on: Jun 27, 2026
 *      Author: tyoung
 */


#include "depth_sensor.h"
#include "usbd_cdc_if.h"
#include <string.h>


// Constants
#define CONVERSION_TIME_MS   18u          // max OSR-8192 conversion time
#define ATMOSPHERIC_PA       101300.0f
#define SEAWATER_DENSITY     1029.0f
#define GRAVITY              9.80665f
#define PACKET_LEN           10u

// Module-private state
static I2C_HandleTypeDef *s_hi2c   = NULL;
static SystemState        *systemState  = NULL;

//should probbaly chnage name from coeff to prom
static uint16_t  coeff[7];         // PROM calibration coefficients C0-C6 - unique to each sensor
static uint8_t   sensor_state = 0; // state-machine position
static uint32_t  tick         = 0; // timestamp of last conversion start
static uint8_t   data[3];          // raw ADC read buffer
static uint32_t  D1           = 0; // raw pressure ADC value
static uint32_t  D2           = 0; // raw temperature ADC value

/**
 * * @brief  CRC-4 check over the 7 PROM words as specified in the MS5837-02BA datasheet.
*/

static uint8_t crc4(uint16_t coeff[]){

	uint16_t n_rem   = 0x00;
    uint16_t crc_buf[8];

    //copy coefficients; mask CRC nibble from word 0
    for (uint8_t i = 0; i < 8; i++)
    {
        crc_buf[i] = (i < 7) ? coeff[i] : 0x0000;
    }
    crc_buf[0] = crc_buf[0] & 0x0FFF; //clear CRC nibble

    for (uint8_t i = 0; i < 16; i++)
    {
        if (i % 2 == 1)
            n_rem ^= (crc_buf[i >> 1] & 0x00FF);
        else
            n_rem ^= (crc_buf[i >> 1] >> 8);

        for (uint8_t n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & 0x8000)
                n_rem = (n_rem << 1) ^ 0x3000;
            else
                n_rem <<= 1;
        }
    }

    n_rem = ((n_rem >> 12) & 0x000F);
    return (n_rem ^ 0x00);
}


/**
 * @brief  Apply first- and second-order MS5837-02BA compensation, then compute
 *         depth.  Pressure and temperature are kept internal to this module.
 *         Only depth is written to SystemState.
 */
static void compute_depth(void){
	//first order compensation

	int32_t dT = (int32_t)D2 - (int32_t)((uint32_t)coeff[5] * 256L);	//difference between actual and reference temperature
	int32_t TEMP = 2000L + (((int64_t)(dT) * coeff[6]) / 8388608LL);

	int64_t OFF = ((int64_t)(coeff[2]) * 131072L) + (((int64_t)(coeff[4]) * dT) / 64L); //pressure offset at actual temperature
	int64_t SENS = (((int64_t)coeff[1]) * 65536L) + (((int64_t)(coeff[3]) * dT) / 128L); //pressure sensitivity at actual temp

	//second order low-temperature correction - sensor values become nonlinear under 20 C

	int32_t Ti    = 0;
	int64_t OFFi  = 0;
	int64_t SENSi = 0;

	if ((TEMP / 100) < 20)
	{
	    Ti    = (int32_t)((11LL * (int64_t)dT * (int64_t)dT) / 34359738368LL);
	    OFFi  = (31LL * (TEMP - 2000LL) * (TEMP - 2000LL)) / 8LL;
	    SENSi = (63LL * (TEMP - 2000LL) * (TEMP - 2000LL)) / 32LL;
	}

	//2nd order corrections
	int64_t OFF2  = OFF  - OFFi;
	int64_t SENS2 = SENS - SENSi;
	int32_t TEMP2 = (TEMP - Ti) / 100;


	//calculate pressure
	int32_t P2 = (int32_t)(((int64_t)D1 * SENS2 / 2097152LL - OFF2) / 32768LL) / 100;

	//convert to depth from pressure reading
	float pressure_pa    = (float)P2 * 100.0f;
	float gauge_pressure = pressure_pa - ATMOSPHERIC_PA;  // subtract 101300 Pa
	systemState->depth       = gauge_pressure / (SEAWATER_DENSITY * GRAVITY);

	/* Debug
	char buf[64];
	sprintf(buf, "D1:%lu D2:%lu Depth:%.4f\r\n", D1, D2, systemState->depth);
	CDC_Transmit_FS((uint8_t*)buf, strlen(buf));
	*/
}


/**
 * @brief  Build and transmit the 10-byte packet to Orin over USB CDC.
 *
 *  Byte  0     : 0xAA  (header)
 *  Byte  1     : greenPressed
 *  Byte  2     : extKillState
 *  Byte  3     : intKillState
 *  Bytes 4-7   : depth (float, little-endian)
 *  Byte  8     : '\r'
 *  Byte  9     : '\n'
 */

static void transmit_packet(void)
{
    uint8_t packet[PACKET_LEN];

    packet[0] = 0xAA;
    packet[1] = systemState->greenPressed;
    packet[2] = systemState->extKillState;
    packet[3] = systemState->intKillState;
    memcpy(&packet[4], &systemState->depth, sizeof(float));
    packet[8] = '\r';
    packet[9] = '\n';

    CDC_Transmit_FS(packet, PACKET_LEN);
}


HAL_StatusTypeDef Depth_Sensor_Init(I2C_HandleTypeDef *hi2c, SystemState *state)
{
    s_hi2c  = hi2c;
    systemState = state;

    //Reset the sensor
    uint8_t cmd = MS5837_RESET;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(s_hi2c,   MS5837_ADDR << 1, &cmd, 1, 5000);
    HAL_Delay(10); // datasheet: >= 2.8 ms reload time

    if (ret != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Read PROM calibration coefficients (C0-C6)
    for (uint8_t i = 0; i < 7; i++)
    {
        uint8_t prom_cmd = MS5837_PROM_READ + (i * 2);
        ret = HAL_I2C_Master_Transmit(s_hi2c, MS5837_ADDR << 1, &prom_cmd, 1, 5000);

        if (ret != HAL_OK) {
        	return HAL_ERROR;
        }

        uint8_t buf[2] = {0};
        ret = HAL_I2C_Master_Receive(s_hi2c, MS5837_ADDR << 1, buf, 2, 5000);
        if (ret != HAL_OK) {
        	return HAL_ERROR;
        }

        coeff[i] = ((uint16_t)buf[0] << 8) | buf[1];
    }

    // CRC check
    uint8_t crc_read       = coeff[0] >> 12;   // upper nibble of C0
    uint8_t crc_calculated = crc4(coeff);

    if (crc_calculated != crc_read)
    {
        return HAL_ERROR;
    }

    //prime the state machine
    sensor_state = 0;
    return HAL_OK;
}



void Depth_Sensor_Update(void){
	uint32_t now = HAL_GetTick();
	uint8_t cmd;

	switch(sensor_state){

	//state 0: kick off D1 pressure conversion
	case 0:
		cmd = MS5837_CONVERT_D1_8192;
		HAL_I2C_Master_Transmit(s_hi2c, MS5837_ADDR << 1, &cmd, 1, 100);
		tick = now;
        sensor_state = 1;
        break;

	case 1:
        if ((now - tick) >= CONVERSION_TIME_MS)
        {
            cmd = MS5837_ADC_READ;
            HAL_I2C_Master_Transmit(s_hi2c, MS5837_ADDR << 1, &cmd, 1, 100);
            HAL_I2C_Master_Receive(s_hi2c, MS5837_ADDR << 1, data, 3, 100);
            D1 = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] <<  8) |  (uint32_t)data[2];

            cmd = MS5837_CONVERT_D2_8192;
            HAL_I2C_Master_Transmit(s_hi2c, MS5837_ADDR << 1, &cmd, 1, 100);
            tick = now;
            sensor_state = 2;
        }
		break;
	case 2:
        if ((now - tick) >= CONVERSION_TIME_MS)
        {
            cmd = MS5837_ADC_READ;
            HAL_I2C_Master_Transmit(s_hi2c, MS5837_ADDR << 1, &cmd, 1, 100);
            HAL_I2C_Master_Receive(s_hi2c, MS5837_ADDR << 1, data, 3, 100);
            D2 = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] <<  8) | (uint32_t)data[2];

            compute_depth();
            transmit_packet();

            //loop back to start next D1 conversion immediately
            sensor_state = 0;
        }
		break;

	default:
		sensor_state = 0;
		break;
	}

}


