/*
 * motor_control.h
 *
 *  Created on: Jun 4, 2026
 *      Author: Trevor Young
 */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

typedef struct SystemState SystemState;

void Motor_Init(SystemState *state);

void Motor_Update(void);

void Motor_Kill(void);

void Motor_Start_Arming(void);


#endif /* INC_MOTOR_CONTROL_H_ */
