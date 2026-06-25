/*
 * safety_manager.h
 *
 *  Created on: Jun 6, 2026
 *      Author: Trevor Young
 */

#ifndef INC_SAFETY_MANAGER_H_
#define INC_SAFETY_MANAGER_H_

#include "motor_control.h"

typedef struct SystemState SystemState;

void Safety_Manager_Update(SystemState *state);

#endif /* INC_SAFETY_MANAGER_H_ */
