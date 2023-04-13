/*
 * eeprom.h
 *
 *  Created on: Oct 14, 2013
 *      Author: mark
 *
 *  Added EEPROM commands to make EEPROM.C more clear by eliminating 'magic' numbers
 *      date: 19 July,2022
 *      author: mw
 *
 */

#ifndef EEPROM_H_
#define EEPROM_H_

void EEPROM_init(void);

void EEPROM_writeLong32(unsigned int addr,long v);
long EEPROM_readLong32(unsigned int addr);

void EEPROM_writeWord16(unsigned int addr,int v);
int EEPROM_readWord16(unsigned int addr);


void EEPROM_writeFloat(unsigned int addr,float v);
float EEPROM_readFloat(unsigned int addr);


#define EE_CALIBRATION 0x000

#define EE_LAST_VERSION 0xFA
#define EE_REFRESH_TIME 0xFB
#define EE_TORQUE_ROUNDING 0xFC
#define EE_MAX_TORQUE 0xFD		//
#define EE_ACTUATION_COUNT 0xFE	//
#define EE_SERIAL_NUMBER 0x0FF	// last word of memory

//EEPROM commands
#define EE_WREN     0x06    //write enable
#define EE_WRDI     0x04    //reset write enable latch
#define EE_RDSD     0x05    //read status register
#define EE_WRSR     0x01    //write status latch
#define EE_READ     0x03    //read memory
#define EE_WRITE    0x02    //write memory

#endif /* EEPROM_H_ */
