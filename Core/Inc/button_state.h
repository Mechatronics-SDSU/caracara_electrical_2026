/*
 * button_state.h
 *
 *  Created on: Jun 17, 2026
 *      Author: tyoung
 */

#ifndef INC_BUTTON_STATE_H_
#define INC_BUTTON_STATE_H_

typedef struct SystemState SystemState;

void Button_Update(SystemState *state);

void Button_TimerCallback(void);


#endif /* INC_BUTTON_STATE_H_ */
