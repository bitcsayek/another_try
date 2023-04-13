/*
 * FieldTorqueInc.c
 *
 *  Created on: Apr 4, 2017
 *      Author: mark
 */
#include "FieldTorqueInc.h"
#include "buttons.h"
#include "states.h"
#include "lcd.h"
#include "eeprom.h"

#define TWO_SECONDS ( 64 )
#define TEN_SECONDS ( 320 )
#define ONE_MINUTE (60*32)

int incrementInc;
int incrementDec;
int setupIncrement;

static int maxFlashTimer;
static int maxPauseTimer;

extern unsigned int setupTimout;
extern int TorqueRounding;

extern void checkAdjustAbort(int abortWithButtons);	// eternal in MaxSetup.c


void initFieldTorqueIncrementStateMachine(int initIncrement)
{
	incrementAdjustState = S_Increment_adjust;
	setupTimout = 0;
	incrementInc = 1;
	incrementDec = 1;
	setupIncrement = initIncrement;
}

void displayIncrement(int targetRefresh)
{
	if ( maxFlashTimer < 16)
	{
		LCD_displayInt(targetRefresh);
	}
	else if ( maxFlashTimer > 31 )
	{
		maxFlashTimer = 0;
	}
}

void displayIncrementAdjust(void)
{

	displayIncrement(setupIncrement);
	maxFlashTimer++;

	if ( current_PT )
	{
		if ( maxFlashTimer == 32 )
		{
			current_PT = 0;

			if ( (setupIncrement % 1000) == 0 )
			{
				incrementInc = 1000;
			}
			else if ( (setupIncrement % 100) == 0 )
			{
				incrementInc = 100;
			}
			else if ( (setupIncrement % 10) == 0 )
			{
				incrementInc = 10;
			}
			else
			{
				incrementInc = 1;
			}
		}
	}
	else
	{
		incrementInc = 1;
	}

	if ( current_EM )
	{
		if ( maxFlashTimer == 32 )
		{
			current_EM = 0;

			if ( (setupIncrement % 1000)== 0 )
			{
				incrementDec = 1000;
			}
			else if ( (setupIncrement % 100) == 0 )
			{
				incrementDec = 100;
			}
			else if ( (setupIncrement % 10) == 0 )
			{
				incrementDec = 10;
			}
			else
			{
				incrementDec = 1;
			}
		}
	}
	else
	{
		incrementDec = 1;
	}



	if ( BUTTON_read_PT_UP () )
	{
		setupIncrement += incrementInc;
		if ( setupIncrement > 1000)
		{
			setupIncrement = 1000;
		}
		maxFlashTimer = 0;

	}

	if ( BUTTON_read_EM_DOWN () )
	{
		setupIncrement -= incrementDec;
		if ( setupIncrement < 1 )
		{
			setupIncrement = 1;
		}
		maxFlashTimer = 0;
	}
}



void FieldTorqueIncrementStateMachine(void)
{
	setupTimout++;

	switch ( incrementAdjustState )
	{
	case S_Increment_adjust:
		displayIncrementAdjust();

		if ( BUTTON_read_IO() )
		{
			maxFlashTimer = 0;
			incrementAdjustState = S_Increment_confirm;
		}
		checkAdjustAbort(0);

		break;
	case S_Increment_confirm:
		displayIncrement(setupIncrement);

		BUTTON_read_IO();
		checkAdjustAbort(1);

		if (current_IO )
		{
			maxPauseTimer = 0;
		}
		else
		{
			maxPauseTimer++;
			if ( maxPauseTimer > TWO_SECONDS )
			{
				maxFlashTimer = 0;
				maxPauseTimer = 0;

				incrementAdjustState = S_Increment_saveMax;
			}
		}
		break;

	case S_Increment_saveMax:

		maxFlashTimer++;

		if ( maxFlashTimer < 16 )
			LCD_save();
		else if ( maxFlashTimer > 31)
			maxFlashTimer = 0;

		if ( BUTTON_read_IO())
		{
			maxFlashTimer = 0;
			maxPauseTimer = 0;
			incrementAdjustState = S_Increment_confirmingMax;

			TorqueRounding = setupIncrement;
			EEPROM_writeLong32(EE_TORQUE_ROUNDING,TorqueRounding);
		}
		else
		{
			checkAdjustAbort(1);
		}
		break;

	case S_Increment_confirmingMax:
		LCD_save();

		if ( BUTTON_read_IO())
		{
			maxPauseTimer = 0;
			maxFlashTimer = 0;
		}
		else
		{
			maxPauseTimer++;
			if ( maxPauseTimer > TWO_SECONDS )
			{
				maxPauseTimer = 0;
				maxFlashTimer = 0;
				incrementAdjustState = S_Increment_adjustOff;
				power_state = P_OFF;
			}
		}
		break;
	}
}
