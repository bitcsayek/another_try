/*
 * FieldDisplayRefresh.c
 *
 *  Created on: Apr 4, 2017
 *      Author: mark
 */

#include "FieldDisplayRefresh.h"
#include "buttons.h"
#include "states.h"
#include "lcd.h"
#include "eeprom.h"

#define TWO_SECONDS ( 64 )
#define TEN_SECONDS ( 320 )
#define ONE_MINUTE (60*32)

int rateInc;
int rateDec;
int setupRate;

static int maxFlashTimer;
static int maxPauseTimer;

extern unsigned int setupTimout;
extern unsigned char RefreshPeriod;

extern void checkAdjustAbort(int abortWithButtons);	// eternal in MaxSetup.c

void initFieldDisplayRefreshStateMachine(int initRefreshRate)
{
	rateAdjustState = S_Rate_adjust;
	setupTimout = 0;
	rateInc = 1;
	rateDec = 1;
	setupRate = initRefreshRate;
}





void displayRefresh(int targetRefresh)
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


void displayRefreshAdjust(void)
{

	displayRefresh(setupRate);
	maxFlashTimer++;

	if ( current_PT )
	{
		if ( maxFlashTimer == 32 )
		{
			current_PT = 0;

			if ( (setupRate % 10) == 0 )
			{
				rateInc = 10;
			}
			else
			{
				rateInc = 1;
			}
		}
	}
	else
	{
		rateInc = 1;
	}

	if ( current_EM )
	{
		if ( maxFlashTimer == 32 )
		{
			current_EM = 0;

			if ( (setupRate % 10) == 0 )
			{
				rateDec = 10;
			}
			else
			{
				rateDec = 1;
			}
		}
	}
	else
	{
		rateDec = 1;
	}



	if ( BUTTON_read_PT_UP () )
	{
		setupRate += rateInc;
		if ( setupRate > 99)
		{
			setupRate = 99;
		}
		maxFlashTimer = 0;

	}

	if ( BUTTON_read_EM_DOWN () )
	{
		setupRate -= rateDec;
		if ( setupRate < 1 )
		{
			setupRate = 1;
		}
		maxFlashTimer = 0;
	}
}


void FieldDisplayRefreshStateMachine(void)
{
	setupTimout++;

	switch ( rateAdjustState )
	{
	case S_Rate_adjust:
		displayRefreshAdjust();

		if ( BUTTON_read_IO() )
		{
			maxFlashTimer = 0;
			rateAdjustState = S_Rate_confirm;
		}
		checkAdjustAbort(0);

		break;
	case S_Rate_confirm:
		displayRefresh(setupRate);

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

				rateAdjustState = S_Rate_saveMax;
			}
		}
		break;

	case S_Rate_saveMax:

		maxFlashTimer++;

		if ( maxFlashTimer < 16 )
			LCD_save();
		else if ( maxFlashTimer > 31)
			maxFlashTimer = 0;

		if ( BUTTON_read_IO())
		{
			maxFlashTimer = 0;
			maxPauseTimer = 0;
			rateAdjustState = S_Rate_confirmingMax;

			RefreshPeriod = setupRate;
			EEPROM_writeLong32(EE_REFRESH_TIME,RefreshPeriod);
		}
		else
		{
			checkAdjustAbort(1);
		}
		break;

	case S_Rate_confirmingMax:
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
				rateAdjustState = S_Rate_adjustOff;
				power_state = P_OFF;
			}
		}
		break;
	}
}
