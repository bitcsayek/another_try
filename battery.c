/*
 * battery.c
 *
 *  Created on: Sep 12, 2013
 *      Author: mark
 *
 *  Added support for LTC3335 Buck/Boost regulator w/I2C Coulomb counter
 *      Date: 30 June, 2022
 *      Author: mw
 */

#include "hwdef.h"
#include "battery.h"
#include "i2c.h"
#include "states.h"


#ifndef __LTC3335
int BATTERY_check(void)
{
	int result;   //return a fully charged battery - fake voltage divider


	// this code not needed when using the coulomb counter LTC3335
	BAT_TS_EN_PORT |= BAT_TS_EN_BIT;

    while (REFCTL0 & REFGENBUSY)
    	;             // If ref generator busy, WAIT

	REFCTL0 |= REFVSEL_3 | REFON ;

    __delay_cycles(75);                      // Delay (~75us) for Ref to settle

	ADC10CTL0 = ADC10SHT_1 | ADC10ON ;
	ADC10CTL1 = ADC10DIV_1 | ADC10SHP ;//| ADC10SSEL_3;
	ADC10CTL2 = ADC10PDIV_1 | ADC10RES | ADC10SR;
	ADC10MCTL0 = ADC10SREF0 | ADC10INCH_0;

	ADC10CTL0 |= ADC10ENC | ADC10SC;

	while (ADC10CTL1 & ADC10BUSY)
		;

	result = ADC10MEM0;


	BAT_TS_EN_PORT &= ~BAT_TS_EN_BIT;

	REFCTL0 &= ~(REFON );

	ADC10CTL0 = 0;

	return result;
//	return 799;//mw
}

#else

/*********************************************************************************************
 * Read/Clear LTC3335 Alarm register
 *
 * Read the alarm register and save value in Alarm array
 * Return 0 upon success or 1 on communication fault
 *
 * Writing a 0 to register E[0] clears alarms
 *
 ********************************************************************************************/

int ReadAlarms(void)
{
    int statval;

    //send start to counter
    i2cm_start( );

    //Select register D to read alarm status
    statval = i2cm_out(COUNTER_ADDR & ADDR_WRITE);
    if(statval == TRUE) return 1;
    statval = i2cm_out(REG_D);
    if(statval == TRUE) return 1;
    i2cm_stop( );

    i2cm_start( );
    statval = i2cm_out(COUNTER_ADDR | ADDR_READ);
    if(statval == TRUE) return 1;
    i2cm_in(Alarm,1);        //get the alarm

    //send stop
    i2cm_stop( );

    return 0;
}

int ClearAlarm(void)
{
    int statval;

    //send start to counter
    i2cm_start( );

    //Select register E to clear alarm status
    statval = i2cm_out(COUNTER_ADDR & ADDR_WRITE);
    if(statval == TRUE) return 1;
    statval = i2cm_out(REG_E);
    if(statval == TRUE) return 1;
    statval = i2cm_out(0x01);   //clear the alarm
    if(statval == TRUE) return 1;
    i2cm_stop( );

    return 0;
}

/*********************************************************************************************
 * Read LTC3335 Coulomb Count
 *
 * Point to register C, Coulomb count register
 * Read the count into the CoulombCount array
 * Test count value to see if it is greater than the cutoff/dead battery value
 * If less than or equal set the power_state variable to P_ON
 *
 * Return 0 if no errors else return 1
 *
 ********************************************************************************************/

int ReadCoulombs(void)
{
    int statval;


    //send start to counter
    i2cm_start( );

    //read count from counter - number of Coulombs burned
    //Select register C - Coulomb counter
    statval = i2cm_out(COUNTER_ADDR & ADDR_WRITE);
    if(statval == TRUE) return 1;
    statval = i2cm_out(REG_C);
    if(statval == TRUE) return 1;
    i2cm_stop( );

    i2cm_start( );
    statval = i2cm_out(COUNTER_ADDR | ADDR_READ);
    if(statval == TRUE) return 1;
    i2cm_in(CoulombCount,1);        //get the coulomb count



    //send stop
    i2cm_stop( );
    if(CoulombCount[0] <= CL_CNT_4 ){
        power_state = P_ON;
    }

    return 0;
}

/*********************************************************************************************
 * Configure LTC3335 registers
 *
 * Set prescale register to 9
 * Set alarm register to dead battery level CL_CNT_5
 *
 * return 0 if all is well
 * return 1 if any communication with LTC3335 is NAK
 * return 2 if alarm is detected
 *
 ********************************************************************************************/


int ConfigCounter(void)
{

    //send start to counter
    i2cm_start( );

    //Write register A - Coulomb counter prescale. Value = 9 from spreadsheet calculations
    i2cm_out(COUNTER_ADDR & ADDR_WRITE);
    i2cm_out(REG_A);
    i2cm_out(0x09);
    i2cm_stop( );

    i2cm_start( );
    i2cm_out(COUNTER_ADDR | ADDR_READ);
    i2cm_out(REG_A);
    i2cm_in(Alarm,1);
    i2cm_stop( );



    //Write register B - alarm threshold
    i2cm_start( );
    i2cm_out(COUNTER_ADDR & ADDR_WRITE);
    i2cm_out(REG_B);
    i2cm_out(0xff);        //default value - determine actual value by test should be close to dead battery count
    i2cm_stop( );

    //read alarm register D should return zero
    i2cm_start( );
    i2cm_out(COUNTER_ADDR & ADDR_WRITE);
    i2cm_out(REG_D);
    i2cm_out(CL_CNT_4);

    i2cm_start( );
    i2cm_out(COUNTER_ADDR | ADDR_READ);
    i2cm_in(Alarm,1);        //get the alarm(s) state
    i2cm_stop( );

    i2cm_start( );
    i2cm_out(COUNTER_ADDR & ADDR_WRITE);
    i2cm_out(REG_C);
    //retval = i2cm_out(0x9A);   //test values comment out for production
    /*  0xBE - 190
        0xB4 - 180
        0x96 - 150
        0x64 - 100
        0x32 - 50
     * */
    //if(retval == TRUE) return retval;
    i2cm_stop( );

    i2cm_start( );
    i2cm_out(COUNTER_ADDR | ADDR_READ);
    i2cm_in(CoulombCount,1);        //get the coulomb count
    i2cm_stop( );

    return 0;   //no faults or alarms
}

void TestCounter(unsigned char state)
{
//Write a 1 to register E to start test mode scope pin 19 to see freq. increase with load
//write a 0 to register E to end test mode and return to normal operation
//Monitor IRQ pin on LTC3335
    i2cm_start( );
    i2cm_out(COUNTER_ADDR & ADDR_WRITE);
    i2cm_out(REG_E);
    i2cm_out(state);
    i2cm_stop( );
}

#endif

//end of file
