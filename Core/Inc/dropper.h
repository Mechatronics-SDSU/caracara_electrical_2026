/*
 * dropper.h
 *
 *  Created on: Jun 25, 2026
 *      Author: tyoung
 */

#ifndef DROPPER_H_
#define DROPPER_H_

typedef struct SystemState SystemState;

void Dropper_Init(SystemState *state);
void Dropper_Update(void);
void Dropper_Kill(void);
void Dropper_Start_Arming(void);


#endif /* DROPPER_H_ */
