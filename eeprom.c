/*
 * eeprom.c
 *
 *  Created on: Oct 14, 2013
 *      Author: mark
 *
 *  incorporated EEPROM commands to eliminate magic numbers in functions
 *  added comments to uncommented code to provide some level of clarity
 *      date: 19 July,2022
 *      author: mw
 */


#include "hwdef.h"
#include "eeprom.h"


void EEPROM_init(void)
{
	UCB0CTLW0 = UCMSB | UCMST | UCMODE_0 | UCSYNC | UCCKPL;

	UCB0BR0 = 4;
	UCB0BR1 = 0;

	UCB0CTL1 = UCSSEL_2 ;
}

void EEPROM_off(void)
{
	UCB0CTL1 = UCSWRST ;
}

unsigned char dummy;
unsigned char status;

unsigned char EEPROM_status(void)
{
	unsigned char status;

	EPROM_PORT &= ~EPROM_CS_B_BIT;

	UCB0TXBUF = EE_RDSD;                //read the status register
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	UCB0TXBUF = 0x00;                   //dummy write to retrieve the status word
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	status = UCB0RXBUF;                 //save status word for return

	EPROM_PORT |= EPROM_CS_B_BIT;

	return status;
}


void EEPROM_writeLong32(unsigned int addr, long v)
{

	unsigned int i;
	unsigned char buff[6];

	addr *= 4;

	EPROM_PORT |= EPROM_WP_B_BIT;   //write enable the part

	buff[0] = addr >> 8;            //build 16 bit target address
	buff[1] = addr & 0xff;

	buff[5] = 0xff & v;
	v >>= 8;
	buff[4] = 0xff & v;
	v >>= 8;
	buff[3] = 0xff & v;
	v >>= 8;
	buff[2] = 0xff & v;

	//wait for device to become ready
	do
	{
		status = EEPROM_status();
	} while ( status & 0x01 );

	EPROM_PORT &= ~EPROM_CS_B_BIT;

	UCB0TXBUF = EE_WREN;				// enable write mode
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	EPROM_PORT |= EPROM_CS_B_BIT;

//wait for device to become ready
	do
	{
		status = EEPROM_status();
	} while ( status & 0x01 );

	EPROM_PORT &= ~EPROM_CS_B_BIT;

	UCB0TXBUF = EE_WRITE;               //send the write command
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	//write the buffer to EEPROM
	for ( i = 0 ; i < 6 ; i++ )
	{

		UCB0TXBUF = buff[i];
		while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
			;
		dummy = UCB0RXBUF;

	}

	EPROM_PORT |= EPROM_CS_B_BIT;

	status = EEPROM_status();           //get status word

	EPROM_PORT &= ~EPROM_CS_B_BIT;

	UCB0TXBUF = EE_WRDI;                //reset the write enable latch
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	EPROM_PORT |= EPROM_CS_B_BIT;
	EPROM_PORT &= ~EPROM_WP_B_BIT;      //write protect the part

	status = EEPROM_status();   //get status word
}

long EEPROM_readLong32(unsigned int addr)
{
	unsigned char buff[6];
	long result;

	addr *= 4;

	do
	{
		status = EEPROM_status();
	} while ( status & 0x01 );

	EPROM_PORT &= ~EPROM_CS_B_BIT;

	buff[0] = addr >> 8;
	buff[1] = addr & 0xff;

	UCB0TXBUF = EE_READ;                //send the read memory command
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	//send 16 bit address to read from
	UCB0TXBUF = buff[0];
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	UCB0TXBUF = buff[1];
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	//dummy transmit to receive data
	UCB0TXBUF = 0;
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	buff[2] = UCB0RXBUF;

	UCB0TXBUF = 0;
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	buff[3] = UCB0RXBUF;

	UCB0TXBUF = 0;
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	buff[4] = UCB0RXBUF;

	UCB0TXBUF = 0;
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	buff[5] = UCB0RXBUF;

	EPROM_PORT |= EPROM_CS_B_BIT;

	result = buff[2];
	result <<= 8;
	result |= buff[3];
	result <<= 8;
	result |= buff[4];
	result <<= 8;
	result |= buff[5];

	return result;

}

void EEPROM_writeWord16(unsigned int addr, int v)
{

	unsigned int i;
	unsigned char buff[6];

	addr *= 4;

	EPROM_PORT |= EPROM_WP_B_BIT;   //write enable the part

	buff[0] = addr >> 8;            //build 16 bit target address
	buff[1] = addr & 0xff;

	buff[3] = 0xff & v;
	v >>= 8;
	buff[2] = 0xff & v;

	//wait for device to become ready
	do
	{
		status = EEPROM_status();
	} while ( status & 0x01 );

	EPROM_PORT &= ~EPROM_CS_B_BIT;

	UCB0TXBUF = EE_WREN;				//enable write mode
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	EPROM_PORT |= EPROM_CS_B_BIT;

//wait for device to become ready
	do
	{
		status = EEPROM_status();
	} while ( status & 0x01 );

	EPROM_PORT &= ~EPROM_CS_B_BIT;

	UCB0TXBUF = EE_WRITE;               //send write enable command
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	//send the data - address first
	for ( i = 0 ; i < 4 ; i++ )
	{

		UCB0TXBUF = buff[i];
		while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
			;
		dummy = UCB0RXBUF;

	}

	EPROM_PORT |= EPROM_CS_B_BIT;

	status = EEPROM_status();           //read status word

	EPROM_PORT &= ~EPROM_CS_B_BIT;

	UCB0TXBUF = EE_WRDI;                //send the write disable command
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	EPROM_PORT |= EPROM_CS_B_BIT;
	EPROM_PORT &= ~EPROM_WP_B_BIT;      //write protect the part

	status = EEPROM_status();           //read status word
}

int EEPROM_readWord16(unsigned int addr)
{
	unsigned char buff[4];
	long result;

	addr *= 4;

//wait for device to become ready
	do
	{
		status = EEPROM_status();
	} while ( status & 0x01 );

	EPROM_PORT &= ~EPROM_CS_B_BIT;

	buff[0] = addr >> 8;                //build the read address
	buff[1] = addr & 0xff;

	UCB0TXBUF = EE_READ;                //send the read memory command
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	UCB0TXBUF = buff[0];
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	UCB0TXBUF = buff[1];
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	dummy = UCB0RXBUF;

	//dummy transmits to read memory
	//read word into bottom 16 bits of 32 bit word
	UCB0TXBUF = 0;
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	buff[2] = UCB0RXBUF;

	UCB0TXBUF = 0;
	while ( UCB0STATW & UCBUSY )        //wait for SPI module to become not busy
		;
	buff[3] = UCB0RXBUF;

	EPROM_PORT |= EPROM_CS_B_BIT;

	result = buff[2];
	result <<= 8;
	result |= buff[3];

	return result;

}

union converter
{
	long l;
	float f;
};


void EEPROM_writeFloat(unsigned int addr,float v)
{
	union converter z;
	z.f = v;
	EEPROM_writeLong32(addr,z.l);
}


float EEPROM_readFloat(unsigned int addr)
{
	union converter z;
	z.l = EEPROM_readLong32(addr);
	return z.f;
}


