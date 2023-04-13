/*
 * buttons.c
 *
 *  Created on: Sep 12, 2013
 *      Author: mark
 */

#include "hwdef.h"
#include "states.h"
#include "uart.h"
#include "lcd.h"
#include "buttons.h"
#include "FieldDisplayRefresh.h"
#include "FieldTorqueInc.h"

int lowPower;

int setupTimout = 0;

extern long unsigned actuationCount;

extern unsigned int calibrationFlashTimer;
extern unsigned int calibrationTimeout;
extern int auto_off_timer_ticks;

extern char RefreshPeriod;
extern int TorqueRounding;

void to_lpm(void);

int current_IO = 0;
int current_PT = 0;
int current_EM = 0;

int BUTTON_read_IO(void)
{
	int result;

	IO_SWITCH_EN_PORT |= IO_SW_IO_EN_BIT;
	__delay_cycles(10);
	result = IO_SW_PORT & IO_IO_SW_BIT;
	IO_SWITCH_EN_PORT &= ~IO_SW_IO_EN_BIT;

	if ( result )
	{
		current_IO = 0;
		return 0;
	}

	if ( !result && !current_IO )
	{
		auto_off_timer_ticks = 0;
		calibrationTimeout = 0;
		current_IO = 1;
		return 1;
	}

	return 0;
}

int BUTTON_IO_timer = 0;
int BUTTON_IO_timer_state = 0;

int BUTTON_timed_read_IO_init(void)
{
	BUTTON_IO_timer_state = 0;
	return 0;
}

int BUTTON_timed_read_IO(void)
{
	int result;

	IO_SWITCH_EN_PORT |= IO_SW_IO_EN_BIT;
	__delay_cycles(10);
	result = IO_SW_PORT & IO_IO_SW_BIT;
	IO_SWITCH_EN_PORT &= ~IO_SW_IO_EN_BIT;


	if ( result && current_IO )	// just released
	{
		current_IO = 0;

		if ( BUTTON_IO_timer_state == 0 )
		{
			BUTTON_IO_timer_state = 1;
			return 0;
		}

		if ( BUTTON_IO_timer < BUTTON_SHORT_LONG_THRESHOLD )
		{
			return BUTTON_SHORT_PRESS;
		}
		else
		{
			return BUTTON_LONG_PRESS;
		}
	}

	if ( result )	// not pressed
	{
		current_IO = 0;
		return 0;
	}


	if ( !result && !current_IO ) // just pressed
	{
		auto_off_timer_ticks = 0;
		calibrationTimeout = 0;
		current_IO = 1;
		BUTTON_IO_timer = 0;
		BUTTON_IO_timer_state = 1;
		return 0;
	}

	if ( !result && current_IO )	// being pressed
	{
		if ( BUTTON_IO_timer_state == 1 )
		{
			BUTTON_IO_timer++;
		}
		return 0;
	}



	return 0;
}



extern int battery_timer;
extern int tickClock;
extern int vbatt;

int BUTTON_wait_for_on(void)
{

	LCD_clear(0);
	LCD_paint();

	LCDCCTL0 &=  ~LCDON ;

//	SD24BCTL0 = 0; // delta sigma off.

	P2IES = 0x00;	// all buttons low to high interrupt.
	P2IFG = 0x00;	// clear any pending flags
	P2IE  = IO_IO_SW_BIT;	// io button enabled.

	IO_SWITCH_EN_PORT |= IO_SW_IO_EN_BIT;	// turn on drive
	__delay_cycles(10);


	while ( !(IO_SW_PORT & IO_IO_SW_BIT ))	// wait for button release
	{
		// wait for interrupt, it could be something else

		P2IFG = 0x00;	// clear any pending flags
		P2IE  = IO_IO_SW_BIT;	// io button enabled.

		TA0CTL &= ~TAIE;						// disable interrupts
		TA0CTL &= ~TAIFG;						// clear any pending timer interrupt.

		__bis_SR_register(LPM3_bits | GIE);     // Enter LPM3, enable interrupts
	    __no_operation();                       // For debugger


		P2IE  = 0x00;	// all disabled.
		P2IFG = 0x00;	// clear any pending flags

	};

	current_IO = 0;

	P2IES = 0x70;	// all buttons high to low interrupt.
	P2IFG = 0x00;	// clear any pending flags
	P2IE  = IO_IO_SW_BIT;	// io button enabled.

	UART_Off();

	TA0CCR0 = 0x3FFF;		// 1/2 second


	do
	{
// wait for interrupt, it could be something else
		TA0CTL &= ~TAIFG;						// clear any pending timer interrupt.
		TA0CTL |= TAIE;							// enable interrupts

		P2IFG = 0x00;	// clear any pending flags
		P2IE  = IO_IO_SW_BIT;	// io button enabled.


		__bis_SR_register(LPM3_bits | GIE);     // Enter LPM3, enable interrupts
	    __no_operation();                       // For debugger

	    lowPower = 1;
	    if (tickClock )
	    {
			TA0CTL &= ~TAIE;						// disable interrupts
			TA0CTL &= ~TAIFG;						// clear any pending timer interrupt.
			tickClock = 0;
#ifndef __LTC3335
			if ( ADPTR_ON_PORT & ADPTR_ON_BIT)
			{
				P2DIR &=  ~0x04;

				LCDCCTL0 |=  LCDON ;

				if ( !(CHARGING_B_PORT & CHARGING_B_BIT) )
				{
					LCD_clear(0);
					LCD_batFrm(1);


					// each tick is 1/2 of a second, so test accordingly

					switch ( battery_timer )
					{
					default:
					case 0:
						battery_timer = 1;
						break;
					case 3:
						LCD_batHi(1);
					case 2:
						LCD_batMid(1);
					case 1:
						LCD_batLow(1);
						battery_timer++;
						break;
					}
				}
				else
				{
					LCD_clear(0);
					LCD_batFrm(1);
					LCD_batHi(1);
					LCD_batMid(1);
					LCD_batLow(1);
				}
			}
		    else
#endif
		    {
		    	LCDCCTL0 &=  ~LCDON ;
				P2DIR |=  0x04;
		    }

			LCD_paint();
	    }


		P2IE  = 0x00;	// all disabled.
		P2IFG = 0x00;	// clear any pending flags

	} while ( IO_SW_PORT & IO_IO_SW_BIT );

	current_IO = 1;

	IO_SWITCH_EN_PORT |= IO_SWS_EN_BIT;
	__delay_cycles(10);

	if ( (IO_SW_PORT & (IO_PT_SW_BIT | IO_EM_SW_BIT )) == 0 )
	{
		IO_SWITCH_EN_PORT &= ~IO_SW_IO_EN_BIT;	// turn off drive
		P2IE  = 0x00;	// all disabled.
		P2IFG = 0x00;	// clear any pending flags

		UART_init();


		LCDCCTL0 |=  LCDON ;

		power_state = P_FIELD_CALIBRATE;
		fieldCalibrationState = S_set20;
		calibrationFlashTimer= 0;
		calibrationTimeout = 0;
		auto_off_timer_ticks = 0;

		LCD_calibration();
		LCD_paint();

		TA0CCR0 = 0x03FF;		// 1/32 second

		tickClock = 0;

		TA0CTL &= ~TAIFG;						// clear any pending timer interrupt.
		TA0CTL |= TAIE;							// enable interrupts



		while ( tickClock < 64 )
		{
			to_lpm();
			;// pause
		}


		current_PT = 1;
		current_EM = 1;

		return 0;	// no version number displayed
	}

	else if ( (IO_SW_PORT & IO_EM_SW_BIT) == 0 ) // check DISPLAY_REFRESH MODE
	{
		IO_SWITCH_EN_PORT &= ~IO_SW_IO_EN_BIT;	// turn off drive
		P2IE  = 0x00;	// all disabled.
		P2IFG = 0x00;	// clear any pending flags

		UART_init();


		LCDCCTL0 |=  LCDON ;

		power_state = P_FIELD_DISPLAY_REFRESH;

		LCD_clear(DISPLAY_ALL_OFF);

		TA0CCR0 = 0x03FF;		// 1/32 second

		tickClock = 0;

		TA0CTL &= ~TAIFG;						// clear any pending timer interrupt.
		TA0CTL |= TAIE;							// enable interrupts

		tickClock = 0;

		initFieldDisplayRefreshStateMachine(RefreshPeriod);

		tickClock = 0;
		current_EM = 1;

		return 0;	// no version number displayed
	}
	else if ( (IO_SW_PORT & IO_PT_SW_BIT) == 0 ) // check for customer config mode . left button
	{
		IO_SWITCH_EN_PORT &= ~IO_SW_IO_EN_BIT;	// turn off drive
		P2IE  = 0x00;	// all disabled.
		P2IFG = 0x00;	// clear any pending flags

		UART_init();


		LCDCCTL0 |=  LCDON ;

		power_state = P_FIELD_TORQUE_INCREMENT;

		LCD_clear(DISPLAY_ALL_OFF);
		LCD_displayFloat0D(22);
		LCD_paint();

		TA0CCR0 = 0x03FF;		// 1/32 second

		tickClock = 0;

		tickClock = 0;

		initFieldTorqueIncrementStateMachine(TorqueRounding);

		tickClock = 0;
		current_PT = 1;

		return 0;	// no version number displayed
	}


	else
	{

		IO_SWITCH_EN_PORT &= ~IO_SW_IO_EN_BIT;	// turn off drive
		P2IE  = 0x00;	// all disabled.
		P2IFG = 0x00;	// clear any pending flags

		TA0CCR0 = 0x03FF;		// 1/32 second

		LCD_clear(DISPLAY_ALL_OFF);
		LCD_paint();

		UART_init();

		LCDCCTL0 |=  LCDON ;

		power_state = P_ON;

		return 1;		// version number is displayed

	}

}



int BUTTON_read_PT_UP(void)
{
	int result;

	IO_SWITCH_EN_PORT |= IO_SWS_EN_BIT;
	__delay_cycles(10);
	result = IO_SW_PORT & IO_PT_SW_BIT;
	IO_SWITCH_EN_PORT &= ~IO_SWS_EN_BIT;

	if ( result )
	{
		current_PT = 0;
		return 0;
	}

	if ( !result && !current_PT )
	{
		calibrationTimeout = 0;
		setupTimout = 0;
		auto_off_timer_ticks = 0;
		current_PT = 1;
		return 1;
	}

	return 0;

}

int BUTTON_read_EM_DOWN(void)
{
	int result;

	IO_SWITCH_EN_PORT |= IO_SWS_EN_BIT;
	__delay_cycles(10);
	result = IO_SW_PORT & IO_EM_SW_BIT;
	IO_SWITCH_EN_PORT &= ~IO_SWS_EN_BIT;

	if ( result )
	{
		current_EM = 0;
		return 0;
	}

	if ( !result && !current_EM )
	{
		calibrationTimeout = 0;
		setupTimout = 0;
		auto_off_timer_ticks = 0;
		current_EM = 1;
		return 1;
	}

	return 0;
}



