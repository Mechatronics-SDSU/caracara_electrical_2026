/*
 * grabber.h
 *
 *  Created on: Jun 24, 2026
 *      Author: tyoung
 */

#ifndef INC_GRABBER_H_
#define INC_GRABBER_H_

typedef struct SystemState SystemState;

void Grabber_Init(SystemState *state);
void Grabber_Update(void);
void Grabber_Kill(void);
void Grabber_Start_Arming(void);



#endif /* INC_GRABBER_H_ */
