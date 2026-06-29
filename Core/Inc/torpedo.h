/*
 * torpedo.h
 *
 *  Created on: Jun 24, 2026
 *      Author: tyoung
 */

#ifndef INC_TORPEDO_H_
#define INC_TORPEDO_H_

typedef struct SystemState SystemState;

void Torpedo_Start_Arming(void);

void Torpedo_Init(SystemState *state);

void Torpedo_Update(void);

#endif /* INC_TORPEDO_H_ */
