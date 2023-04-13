/*
 * MaxSetup.h
 *
 *  Created on: Sep 7, 2015
 *      Author: mark
 */

#ifndef MAXSETUP_H_
#define MAXSETUP_H_

extern int Max_Torque;

void initMaxSetupStateMachine(int initSetupTorque);
void MaxSetupStateMachine(void);


#endif /* MAXSETUP_H_ */
