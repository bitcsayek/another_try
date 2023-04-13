/*
 * MaxSetup.c
 *
 *  Created on: Sep 7, 2015
 *      Author: mark
 */

#include <stdlib.h>
#include <math.h>

#include "hwdef.h"
#include "lcd.h"
#include "eeprom.h"
#include "buttons.h"
#include "MaxSetup.h"
#include "states.h"



#define TWO_SECONDS ( 64 )
#define TEN_SECONDS ( 320 )
#define ONE_MINUTE (60*32)


extern unsigned int setupTimout;

static int maxFlashTimer;
static int maxPauseTimer;
int setupTorque;

static int torqueInc = 25;
static int torqueDec = 25;

void displaySetupTorque(int targetTorque)
{
	LCD_ftlbs(1);

	if ( maxFlashTimer < 16)
	{
		LCD_displayInt(targetTorque);
	}
	else if ( maxFlashTimer > 31 )
	{
		maxFlashTimer = 0;
	}
}




void displaySetupAdjustTorque(void)
{

	displaySetupTorque(setupTorque);
	maxFlashTimer++;

	if ( current_PT )
	{
		if ( maxFlashTimer == 32 )
		{
			current_PT = 0;

			if ( (setupTorque % 1000) == 0 )
			{
				torqueInc = 1000;
			}
			else if ( (setupTorque % 100) == 0 )
			{
				torqueInc = 100;
			}
			else
			{
				torqueInc = 25;
			}
		}
	}
	else
	{
		torqueInc = 25;
	}

	if ( current_EM )
	{
		if ( maxFlashTimer == 32 )
		{
			current_EM = 0;

			if ( (setupTorque % 1000)== 0 )
			{
				torqueDec = 1000;
			}
			else if ( (setupTorque % 100) == 0 )
			{
				torqueDec = 100;
			}
			else
			{
				torqueDec = 25;
			}
		}
	}
	else
	{
		torqueDec = 25;
	}

	if ( BUTTON_read_PT_UP () )
	{
		setupTorque += torqueInc;
		if ( setupTorque > 9000)
		{
			setupTorque = 9000;
		}
		maxFlashTimer = 0;

	}

	if ( BUTTON_read_EM_DOWN () )
	{
		setupTorque -= torqueDec;
		if ( setupTorque < 100 )
		{
			setupTorque = 100;
		}
		maxFlashTimer = 0;
	}
}




void checkAdjustAbort(int abortWithButtons)
{
	if ( abortWithButtons )
	{
		if ( BUTTON_read_PT_UP () )
		{
			power_state = P_OFF;
		}
		if ( BUTTON_read_EM_DOWN () )
		{
			power_state = P_OFF;
		}
	}
	if ( setupTimout > ONE_MINUTE)
	{
		power_state = P_OFF;
	}
}

void initMaxSetupStateMachine(int initSetupTorque)
{
	maxAdjustState = S_adjust;
	setupTimout = 0;
	torqueInc = 25;
	torqueDec = 25;
	setupTorque = initSetupTorque;
}

void MaxSetupStateMachine(void)
{
	setupTimout++;

	switch ( maxAdjustState )
	{
	case S_adjust:
		displaySetupAdjustTorque();

		if ( BUTTON_read_IO() )
		{
			maxFlashTimer = 0;
			maxAdjustState = S_confirm;
		}
		checkAdjustAbort(0);

		break;
	case S_confirm:
		displaySetupTorque(setupTorque);

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

				maxAdjustState = S_saveMax;
			}
		}
		break;

	case S_saveMax:

		maxFlashTimer++;

		if ( maxFlashTimer < 16 )
			LCD_save();
		else if ( maxFlashTimer > 31)
			maxFlashTimer = 0;

		if ( BUTTON_read_IO())
		{
			maxFlashTimer = 0;
			maxPauseTimer = 0;
			maxAdjustState = S_confirmingMax;

			Max_Torque = setupTorque;
			EEPROM_writeLong32(EE_MAX_TORQUE,Max_Torque);
		}
		else
		{
			checkAdjustAbort(1);
		}
		break;

	case S_confirmingMax:
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
				maxAdjustState = S_adjustOff;
				power_state = P_OFF;
			}
		}
		break;
	}
}
