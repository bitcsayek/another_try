/*
 *  Original Bit-Bang code by Dan Bloomquist
 *
 *  Modified GPIO defines to accommodate our board design
*/

/*
use i2cm_out(...) and i2cm_in(...) between calls to i2cm_start(...) and i2cm_stop(...)

typical use(...) 
{
//this is an example of reading the config register of a DS1631
//in this case 'device' is a select number of 0000xxx0

	i2cm_start( );
	
	if( i2cm_out( 0x90 | device ) ) //send write control byte
		return;
	
	else if( i2cm_out( 0xac ) )//send config command byte
		return;
	
	i2cm_start( );//device wants start again to read
	
	if( i2cm_out( 0x91 | device ) ) //send read control byte
		return;
	
	i2cm_in( buf, 1 ); //input the config byte to 'buf'

	i2cm_stop( );
	//device was not busy or failed...
	//set success flag if needed before return
	
} typical use end......

i2cm_out(...) returns true if device did not ack output byte
with mspgcc compiled for optimum code size this is about 256 bytes

*/

//Defines the port to be used
#define I2C_MASTER_REN P3REN
#define I2C_MASTER_DIR P3DIR
#define I2C_MASTER_OUT P3OUT
#define I2C_MASTER_IN  P3IN
//port pins
#define IC2_MASTER_SCL BIT2
#define I2C_MASTER_SDA BIT3

//Declarations
//sends a start condition
//will set sda and scl high and delay before start
void i2cm_start( void );

//send stop condition
//will set sda low before delay and stop
void i2cm_stop( void );

//Output one byte
//assumes sda and scl low and leaves sda, scl low if ack.
//returns true if no ack from device
unsigned char i2cm_out( register unsigned int data );

//input count of bytes into buf[ ]
//Assumes scl low and leaves scl low
//sends ack to device until last byte then no ack
void i2cm_in( unsigned char* buf, int count );
