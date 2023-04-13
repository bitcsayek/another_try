/*
 * deltaSigma.c
 *
 *  Created on: Oct 2, 2013
 *      Author: mark
 */

#include "hwdef.h"



#define noCAL_STUFF


#define FILTER 8

signed long Delta_ReadFinish(void)
{
	unsigned int i;
	signed long result[FILTER];
#if defined(CAL_STUFF)
	signed long calresult[FILTER];
#endif
	signed long result_filter;
//	signed long calresult_filter;

	SEN_EN_PORT |= SEN_EN_BIT;

	result_filter = 0;
//  calresult_filter = 0;

	SD24BCTL0 = /*SD24DIV1 | */ SD24PDIV_0| SD24SSEL_1 | SD24REFS;	// clock at 1Mhz
	SD24BINCTL0 = SD24GAIN_32; // | SD24INTDLY_3;		// gain of 32, forth sample causes interrupt.
	SD24BOSR0 = 0x007f;		// over sample by 128. this gives us 21 bits

	// give us ~ 2ms to let things settle
	__delay_cycles(2000);

	for ( i = 0 ; i < FILTER ; i++)
	{
		SD24BCCTL0 = SD24SNGL | SD24DF0 ;
		SD24BIFG = 0;		  // clear any pending bits.
		SD24BCCTL0 |= SD24SC; // start conversion

		while ( ! (SD24BIFG & SD24IFG0) )	// wait for conversion
			;

		result[i] = SD24BMEMH0;
		result[i] <<= 16;
		result[i] |= SD24BMEML0;

		result_filter += result[i];
	}

#define noCAL_STUFF
#if defined(CAL_STUFF)
	__delay_cycles(1000);

	for ( i = 0 ; i< FILTER ; i++)
	{
		SD24BCCTL0 = SD24SNGL | SD24DF0 | SD24CAL ;
		SD24BIFG = 0;			// clear any pending bits.
		SD24BCCTL0 |= SD24SC; // start conversion

		while ( ! (SD24BIFG & SD24IFG0) )
			;

		calresult[i] = SD24BMEMH0;
		calresult[i] <<= 16;
		calresult[i] |= SD24BMEML0;

		calresult_filter += calresult[i];
	}

#endif

	SEN_EN_PORT &= ~SEN_EN_BIT;

	SD24BCTL0 &= ~SD24REFS;	// reference off. saves juice.

	// the ADC is setup as a +/- 21 bits. (i.e. 22 bits)
	// return the value as a 16 bit number...( 2's complement
//	return (result_filter-calresult_filter)/(64*FILTER);
	return result_filter/(64*FILTER);
}
