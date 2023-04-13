/*
 *  Original Bit-Bang code by Dan Bloomquist
 *
 *  Modified GPIO defines to accommodate our board design
*/

#include "hwdef.h"
#include "I2C.h"

/****************************************************************
static void __inline__ brief_pause( register unsigned int n )
{
__asm__ __volatile__ (
		"1: \n"
		" dec %[n] \n"
		" jne 1b \n" : [n] "+r"(n)
		);
}
****************************************************************/

//Send data byte out on bang i2c, return false if ack
//Assumes start has been set up or a next byte
//so  both lines are assumed low
// **Lower byte of data is sent**
unsigned char i2cm_out( register unsigned int data )
{
	volatile unsigned int i= 0; //will be register
	//output eight bits of 'data'
	for( ; i < 8; ++i )
	{
		//send the data bit starting with MSB
		if( data & 0x80 )
			I2C_MASTER_OUT|= I2C_MASTER_SDA;
		else
			I2C_MASTER_OUT&= ~I2C_MASTER_SDA;
		
		//Set Clock High
		I2C_MASTER_OUT|= IC2_MASTER_SCL;
		
		//Set Clock Low
	    __delay_cycles(0x04);
//		brief_pause( 0x04 );
		I2C_MASTER_OUT&= ~IC2_MASTER_SCL;
		
		//shift next data bit
		data= data << 1;
	}

	I2C_MASTER_DIR&= ~I2C_MASTER_SDA;

	//Set Clock High
	I2C_MASTER_OUT|= IC2_MASTER_SCL;

	//get the ack bit and leave sda in last state
	unsigned char ack= I2C_MASTER_IN & I2C_MASTER_SDA;
	if( ack )
		I2C_MASTER_OUT|= I2C_MASTER_SDA;
	else
		I2C_MASTER_OUT&= ~I2C_MASTER_SDA;

	//take the pin back for output
	I2C_MASTER_DIR|= I2C_MASTER_SDA;

	//Set Clock Low
	I2C_MASTER_OUT&= ~IC2_MASTER_SCL;

	return ack;
}

//Assumes the IC2_MASTER_SCL is low
void i2cm_in( unsigned char* buf, int count )
{
	unsigned char data;
	for( ; count--; )
	{
		data= 0;
		I2C_MASTER_DIR&= ~I2C_MASTER_SDA;
		volatile unsigned int i= 0;
		//read data in starting with MSB
		for( ; i < 8; ++i )
		{
			//Set Clock High
			I2C_MASTER_OUT|= IC2_MASTER_SCL;
			
			//shift the bit over
			data= data << 1;
		
			if( I2C_MASTER_IN & I2C_MASTER_SDA )
				data|= 0x01;
			
	        //__delay_cycles(0x04);
			//Set Clock Low
			I2C_MASTER_OUT&= ~IC2_MASTER_SCL;
		}
		//put the input data byte into the buffer, inc buffer pointer
		*buf++= data;
		
		//No Ack after last byte
		if( count )
			I2C_MASTER_OUT&= ~I2C_MASTER_SDA;
		else
			I2C_MASTER_OUT|= I2C_MASTER_SDA;

		//take sda to output ack
		I2C_MASTER_DIR|= I2C_MASTER_SDA;

		//Set Clock High
		I2C_MASTER_OUT|= IC2_MASTER_SCL;
		
		//Set Clock Low
		__delay_cycles(0x04);
//		brief_pause( 0x04 );
		I2C_MASTER_OUT&= ~IC2_MASTER_SCL;

	}
}

void i2cm_start( void )
{
	I2C_MASTER_OUT|= I2C_MASTER_SDA;
	I2C_MASTER_OUT|= IC2_MASTER_SCL;
	
    __delay_cycles(0x20);
//    brief_pause( 0x20 );
	I2C_MASTER_OUT&= ~I2C_MASTER_SDA;
	
    __delay_cycles(0x20);
//    brief_pause( 0x20 );
	I2C_MASTER_OUT&= ~IC2_MASTER_SCL;
}

//Assumes the clock is low
void i2cm_stop( void )
{
	I2C_MASTER_OUT&= ~I2C_MASTER_SDA;
	
    __delay_cycles(0x20);
//    brief_pause( 0x20 );
	I2C_MASTER_OUT|= IC2_MASTER_SCL;
	
    __delay_cycles(0x20);
//    brief_pause( 0x20 );
	I2C_MASTER_OUT|= I2C_MASTER_SDA;
}
