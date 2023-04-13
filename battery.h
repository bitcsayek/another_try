/*
 * battery.h
 *
 *  Created on: Sep 12, 2013
 *      Author: mark
 *
 *  Modified to accommodate LTC3335 Buck/Boost regulator w/I2C Coulomb counter
 *      Date: 30 June, 2022
 *      Author: mw
 */

#ifndef BATTERY_H_
#define BATTERY_H_

#ifndef __LTC3335
int BATTERY_check(void);
#else

unsigned char CoulombCount[4];
unsigned char Alarm[4];

//LTC3335 configuration registers
#define COUNTER_ADDR    0xC8    //base address 0b1100 100x
#define REG_A           0x01    //Vout select (high nibble) & Coulomb count prescale (low nibble) - Write Only
#define REG_B           0x02    //alarm threshold Register - Write Only
#define REG_C           0x03    //accumulated charge - Read/Write
#define REG_D           0x04    //alarms - Read Only
#define REG_E           0x05    //interrupt request - Write Only

#define ADDR_WRITE      0xFE
#define ADDR_READ       0x01

int ConfigCounter(void);
int ReadCoulombs(void);
void TestCounter(unsigned char state);
int ReadAlarms(void);

#endif

#endif /* BATTERY_H_ */
