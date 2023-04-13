/*
 * calibrate.c
 *
 *  Created on: Oct 23, 2013
 *      Author: mark
 */
#include <stdlib.h>
#include <math.h>

#include "hwdef.h"
#include "timeconst.h"
#include "lcd.h"
#include "eeprom.h"
#include "buttons.h"
#include "calibrate.h"
#include "states.h"
#include "MaxSetup.h"


extern int auto_off_timer_ticks;

unsigned int calibrationFlashTimer;
unsigned int calibrationPauseTimer;
unsigned int calibrationTakenTimer;

extern float pressure;
extern float torque;

extern long calibration_fds;

float calibrationTorque;
int calibrationTimeout = 0;

int number_points;

int loadPoints(void)
{
	unsigned int sensorIndex;

	number_points = 0;


	sensorIndex = 0;
	for ( sensorIndex = 0 ; sensorIndex < NUM_CAL_POINTS ; sensorIndex++)
	{
		pointlist[sensorIndex].count = EEPROM_readWord16(EE_CALIBRATION+sensorIndex*3+0);
		pointlist[sensorIndex].pressure = EEPROM_readFloat(EE_CALIBRATION+sensorIndex*3+1);
		pointlist[sensorIndex].torque   = EEPROM_readFloat(EE_CALIBRATION+sensorIndex*3+2);
	}

	for ( sensorIndex = 0 ; sensorIndex < NUM_CAL_POINTS ; sensorIndex++)
	{
		if ( isnan(pointlist[sensorIndex].pressure) || isnan(pointlist[sensorIndex].torque))
			break;
	}

	number_points = sensorIndex;

	if ( number_points == 0 )
	{
		pointlist[0].count = 0;
		pointlist[0].pressure = 0.0;
		pointlist[0].torque   = 0.0;

		pointlist[1].count = 31000;
		pointlist[1].pressure = 100.0;
		pointlist[1].torque   = 200.0;

		number_points = 2;
	}


	return 1;
}

void ClearCalibration(void)
{
	unsigned int sensorIndex;

	sensorIndex = 0;
	for ( sensorIndex = 0 ; sensorIndex < NUM_CAL_POINTS ; sensorIndex++)
	{
		EEPROM_writeWord16(EE_CALIBRATION+sensorIndex*3+0,0xffff);
		EEPROM_writeLong32(EE_CALIBRATION+sensorIndex*3+1,0xffffffff);
		EEPROM_writeLong32(EE_CALIBRATION+sensorIndex*3+2,0xffffffff);
	}
}




float ConvertFromCounts(long counts, int type, int limitZero)
{
	// type = 0 for pressure
	// type = 1 for torque

	unsigned int lowerIndex;
	unsigned int upperIndex;
	float result;

	lowerIndex = 0;
	upperIndex = 1;

	for ( ; upperIndex < number_points-1 ; upperIndex++)
	{
		if (counts < pointlist[upperIndex].count )
			break;
	}

	lowerIndex = upperIndex -1;

	float offset;
	float slope;

	if ( type == 0 )
	{
		offset = pointlist[lowerIndex].pressure;
		slope =  (pointlist[upperIndex].pressure - pointlist[lowerIndex].pressure)/
				(float)(pointlist[upperIndex].count-pointlist[lowerIndex].count);

		result = offset+(counts - pointlist[lowerIndex].count)*slope;

		if ( limitZero)
		{
			if ( result < ( pointlist[lowerIndex].pressure / 4.0 ))
			{
				result = 0.0;
			}
		}

	}
	if ( type == 1 )
	{
		offset = pointlist[lowerIndex].torque;
		slope =  (pointlist[upperIndex].torque - pointlist[lowerIndex].torque)/
				(float)(pointlist[upperIndex].count-pointlist[lowerIndex].count);

		result = offset+(counts - pointlist[lowerIndex].count)*slope;

		if ( limitZero)
		{
			if ( result < ( pointlist[0].torque / 4.0 ))
			{
				result = 0.0;
			}
		}

	}


    return result;
	//return 500;     //mw for debug purposes ONLY!
}


static const int calibrationADCtarget[NUM_CAL_POINTS] =
{
		5767,	// 20
		11534,	// 40
		17302,	// 60
		23069,	// 80
		25953,	// 90
};
/*	version 1.04
static int calbirationADCwindow[NUM_CAL_POINTS] =
{
		578,
		1153,
		1730,
		2307,
		2883,
};
*/
// version 1.05
static const int calibrationADCwindow[NUM_CAL_POINTS] =
{
		1153,	// 20 psi +/- 20.0%
		2307,	// 40 psi +/- 20.0%
		2595,	// 60 psi +/- 15.0%
		2307,	// 80 psi +/- 10.0%
		2595,	// 90 psi +/- 10.0%
};



int calibrationSanityIndexCheck(unsigned int sensorIndex)
{

	if ( abs ( calPointList[sensorIndex].count - calibrationADCtarget[sensorIndex] ) > calibrationADCwindow[sensorIndex] )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

int calibrationSanityCheck(void)
{
	unsigned int sensorIndex;

	for ( sensorIndex = 0 ; sensorIndex < NUM_CAL_POINTS ; sensorIndex++ )
	{
		if ( !calibrationSanityIndexCheck(sensorIndex) )
			return FALSE;
	}

	return TRUE;
}



void displayCalibrationPressure(float targetPressure)
{
	switch ( pressure_units )
	{
	case D_PSI:
		LCD_psi(1);

		if ( calibrationFlashTimer < 16)
		{
			LCD_displayFloat1D(targetPressure);
		}
		else if ( calibrationFlashTimer > 31 )
		{
			calibrationFlashTimer = 0;
		}
		break;

	case D_BAR:
		LCD_bar(1);

		targetPressure *= PSI2BAR;

		if ( calibrationFlashTimer < 16)
		{
			LCD_displayFloat3D(targetPressure);
		}
		else if ( calibrationFlashTimer > 31 )
		{
			calibrationFlashTimer = 0;
		}

		break;

	case D_KPA:
		LCD_kPa(1);


		targetPressure *= PSI2KPA;

		if ( calibrationFlashTimer < 16)
		{
			LCD_displayFloat1D(targetPressure);
		}
		else if ( calibrationFlashTimer > 31 )
		{
			calibrationFlashTimer = 0;
		}

		break;
	}
}




void displayCalibrationTorque(float targetTorque)
{
	switch ( torque_units )
	{
	case D_NM:
		LCD_nm(1);

		targetTorque *= FTLB2NM;

		if ( calibrationFlashTimer < 16)
		{
			LCD_displayFloat1D(targetTorque);
		}
		else if ( calibrationFlashTimer > 31 )
		{
			calibrationFlashTimer = 0;
		}
		break;

	case D_FTLBS:
		LCD_ftlbs(1);

		if ( calibrationFlashTimer < 16)
		{
			LCD_displayFloat1D(targetTorque);
		}
		else if ( calibrationFlashTimer > 31 )
		{
			calibrationFlashTimer = 0;
		}
		break;
	}


}



static float torqueInc = 1.0;
static float torqueDec = 1.0;


void	displayAdjustTorque(void)
{
	if ( calibrationTakenTimer > 0 )
	{
		LCD_clear(DISPLAY_ALL_ON);
		LCD_paint();
		calibrationTakenTimer--;
		return;
	}

	displayCalibrationTorque(calibrationTorque);
	calibrationFlashTimer++;

	if ( current_PT )
	{
		if ( calibrationFlashTimer == 32 )
		{
			int round;

			current_PT = 0;

			round =  rintf(calibrationTorque);
			if ( (round % 100) == 0 )
			{
				torqueInc = 100.0;
			}
			else if ( (round % 10) == 0 )
			{
				torqueInc = 10.0;
			}
			else
			{
				torqueInc = 1.0;
			}
		}
	}
	else
	{
		torqueInc = 1.0;
	}

	if ( current_EM )
	{
		if ( calibrationFlashTimer == 32 )
		{
			int round;

			current_EM = 0;

			round =  rintf(calibrationTorque);
			if ( (round % 100) == 0 )
			{
				torqueDec = 100.0;
			}
			else if ( (round % 10) == 0 )
			{
				torqueDec = 10.0;
			}
			else
			{
				torqueDec = 1.0;
			}
		}
	}
	else
	{
		torqueDec = 1.0;
	}

	if ( BUTTON_read_PT_UP () )
	{
		calibrationTorque += torqueInc;
		if ( calibrationTorque > 9999.0)
		{
			calibrationTorque = 9999.0;
		}
		calibrationFlashTimer = 0;

	}

	if ( BUTTON_read_EM_DOWN () )
	{
		calibrationTorque -= torqueDec;
		if ( calibrationTorque < 0.0 )
		{
			calibrationTorque = 0.0;
		}
		calibrationFlashTimer = 0;
	}

}

void checkAbort(int abortWithButtons)
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
#if !defined(NO_CALIBRATION_TIMEOUT)
	if ( calibrationTimeout > CAL_TIMEOUT)
	{
		power_state = P_OFF;
	}
#endif
}


typedef struct
{
	long fds;
	float calibrationTorque;
} CalibrationPoint;


CalibrationPoint CalibrationPointsList[10];
int CalibrationIndex;
int CalibrationPointCount;



void CalibrateInitAverage(void)
{
	CalibrationIndex = 0;
	CalibrationPointCount = 0;
	calibrationTakenTimer = 0;
}

void CalibrateCollectAverage(void)
{
	LCD_clear(DISPLAY_ALL_ON);
	LCD_paint();
	calibrationTakenTimer = 8;

	CalibrationPointsList[CalibrationIndex].fds = calibration_fds;
	CalibrationPointsList[CalibrationIndex].calibrationTorque = calibrationTorque;

	CalibrationIndex ++;
	if ( CalibrationIndex >= 10 )
	{
		CalibrationIndex = 0;
	}

	CalibrationPointCount++;
	if ( CalibrationPointCount > 10 )
	{
		CalibrationPointCount = 10;
	}

}
void CalibrateFinishAverage(int index,float pressure)
{
	int i;
	long sum_fds;
	float sum_torq;

	CalibrateCollectAverage();

	sum_fds = 0;
	sum_torq = 0.0;

	for ( i = 0 ; i < CalibrationPointCount ; i++)
	{
		sum_fds += CalibrationPointsList[i].fds;
		sum_torq += CalibrationPointsList[i].calibrationTorque;
	}

	calPointList[index].pressure = pressure;
	calPointList[index].count = sum_fds/CalibrationPointCount;
	calPointList[index].torque = sum_torq/CalibrationPointCount;

}


void FieldCalibrationStateMachine(void)
{
	int button_io_status;
	calibrationTimeout++;
	auto_off_timer_ticks = 0;	// no auto timeout when calibrating


	switch (fieldCalibrationState )
	{

	case S_set20:

		displayCalibrationPressure(20.0);
		calibrationFlashTimer++;
		CalibrateInitAverage();

		if ( BUTTON_read_IO())
		{
			calibrationPauseTimer = 0;
			calibrationFlashTimer = 0;

			calPointList[0].pressure = 20.0;
			calPointList[0].count = calibration_fds;

			if ( calibrationSanityIndexCheck(0))
			{
				fieldCalibrationState = S_confirm20;
			}
			else
			{
				fieldCalibrationState = S_error;
			}
		}

		checkAbort(1);
		break;

	case S_confirm20:
		displayCalibrationPressure(20.0);

		BUTTON_read_IO();
		checkAbort(1);

		if (current_IO )
		{
			calibrationPauseTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;

//				calibrationTorque = pointlist[0].torque;
//				calibrationTorque = 600;
				calibrationTorque = Max_Torque/5.0;

				BUTTON_timed_read_IO_init();
				fieldCalibrationState = S_torque20;
			}
		}
		break;

	case S_torque20:

		displayAdjustTorque();

		if ( button_io_status = BUTTON_timed_read_IO() )
		{
			if ( button_io_status == 1 )
			{
				CalibrateCollectAverage();
			}
			else
			{
				CalibrateFinishAverage(0,20.0);

				calibrationPauseTimer = 0;
				calibrationFlashTimer = 0;
				fieldCalibrationState = S_confirmTorque20;
			}
		}
		checkAbort(0);
		break;

	case S_confirmTorque20:
		displayCalibrationTorque(calibrationTorque);

		BUTTON_read_IO();
		checkAbort(0);
		if (current_IO )
		{
			calibrationFlashTimer = 0;
			calibrationPauseTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;
				fieldCalibrationState = S_set40;
			}
		}
		break;


	case S_set40:
		displayCalibrationPressure(40.0);
		calibrationFlashTimer++;
		CalibrateInitAverage();

		if ( BUTTON_read_IO())
		{
			calibrationPauseTimer = 0;
			calibrationFlashTimer = 0;

			calPointList[1].pressure = 40.0;
			calPointList[1].count = calibration_fds;

			if ( calibrationSanityIndexCheck(1))
			{
				fieldCalibrationState = S_confirm40;
			}
			else
			{
				fieldCalibrationState = S_error;
			}
		}
		checkAbort(1);

		break;

	case S_confirm40:
		displayCalibrationPressure(40.0);

		BUTTON_read_IO();
		checkAbort(1);
		if (current_IO )
		{
			calibrationFlashTimer = 0;
			calibrationPauseTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;

//				calibrationTorque = pointlist[1].torque;
				calibrationTorque = calPointList[0].torque * 2;

				BUTTON_timed_read_IO_init();
				fieldCalibrationState = S_torque40;

			}
		}

		break;

	case S_torque40:
		displayAdjustTorque();

		if ( button_io_status = BUTTON_timed_read_IO() )
		{
			if ( button_io_status == 1 )
			{
				CalibrateCollectAverage();
			}
			else
			{
				CalibrateFinishAverage(1,40.0);

				calibrationPauseTimer = 0;
				calibrationFlashTimer = 0;
				fieldCalibrationState = S_confirmTorque40;
			}
		}
		checkAbort(0);

		break;

	case S_confirmTorque40:
		displayCalibrationTorque(calibrationTorque);

		BUTTON_read_IO();
		checkAbort(0);

		if (current_IO )
		{
			calibrationFlashTimer = 0;
			calibrationPauseTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;
				fieldCalibrationState = S_set60;
			}
		}
		break;


	case S_set60:
		displayCalibrationPressure(60.0);
		calibrationFlashTimer++;
		CalibrateInitAverage();

		if ( BUTTON_read_IO())
		{
			calibrationPauseTimer = 0;
			calibrationFlashTimer = 0;

			calPointList[2].pressure = 60.0;
			calPointList[2].count = calibration_fds;

			if ( calibrationSanityIndexCheck(2))
			{
				fieldCalibrationState = S_confirm60;
			}
			else
			{
				fieldCalibrationState = S_error;
			}
		}
		checkAbort(1);
		break;

	case S_confirm60:
		displayCalibrationPressure(60.0);

		BUTTON_read_IO();
		checkAbort(1);
		if (current_IO )
		{
			calibrationFlashTimer = 0;
			calibrationPauseTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;

//				calibrationTorque = pointlist[2].torque;
				calibrationTorque = calPointList[0].torque * 3;

				BUTTON_timed_read_IO_init();
				fieldCalibrationState = S_torque60;
			}
		}

		break;

	case S_torque60:
		displayAdjustTorque();

		if ( button_io_status = BUTTON_timed_read_IO() )
		{
			if ( button_io_status == 1 )
			{
				CalibrateCollectAverage();
			}
			else
			{
				CalibrateFinishAverage(2,60.0);

				calibrationPauseTimer = 0;
				calibrationFlashTimer = 0;
				fieldCalibrationState = S_confirmTorque60;
			}
		}
		checkAbort(0);

		break;

	case S_confirmTorque60:
		displayCalibrationTorque(calibrationTorque);

		BUTTON_read_IO();
		checkAbort(0);

		if (current_IO )
		{
			calibrationFlashTimer = 0;
			calibrationPauseTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;
				fieldCalibrationState = S_set80;
			}
		}

		break;


	case S_set80:
		displayCalibrationPressure(80.0);
		calibrationFlashTimer++;
		CalibrateInitAverage();

		if ( BUTTON_read_IO())
		{
			calibrationPauseTimer = 0;
			calibrationFlashTimer = 0;

			calPointList[3].pressure = 80.0;
			calPointList[3].count = calibration_fds;

			if ( calibrationSanityIndexCheck(3))
			{
				fieldCalibrationState = S_confirm80;
			}
			else
			{
				fieldCalibrationState = S_error;
			}
		}
		checkAbort(1);
		break;

	case S_confirm80:
		displayCalibrationPressure(80.0);

		BUTTON_read_IO();
		checkAbort(1);
		if (current_IO )
		{
			calibrationPauseTimer = 0;
			calibrationPauseTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;

				BUTTON_timed_read_IO_init();
				calibrationTorque = calPointList[0].torque * 4;
				fieldCalibrationState = S_torque80;
			}
		}
		break;

	case S_torque80:
		displayAdjustTorque();

		if ( button_io_status = BUTTON_timed_read_IO() )
		{
			if ( button_io_status == 1 )
			{
				CalibrateCollectAverage();
			}
			else
			{
				CalibrateFinishAverage(3,80.0);

				calibrationPauseTimer = 0;
				calibrationFlashTimer = 0;
				fieldCalibrationState = S_confirmTorque80;
			}
		}
		checkAbort(0);

		break;

	case S_confirmTorque80:
		displayCalibrationTorque(calibrationTorque);

		BUTTON_read_IO();
		checkAbort(0);

		if (current_IO )
		{
			calibrationFlashTimer = 0;
			calibrationPauseTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;
				fieldCalibrationState = S_set90;
			}
		}
		break;


	case S_set90:
		displayCalibrationPressure(90.0);
		calibrationFlashTimer++;
		CalibrateInitAverage();

		if ( BUTTON_read_IO())
		{
			calibrationPauseTimer = 0;
			calibrationFlashTimer = 0;

			calPointList[4].pressure = 90.0;
			calPointList[4].count = calibration_fds;

			if ( calibrationSanityIndexCheck(4))
			{
				fieldCalibrationState = S_confirm90;
			}
			else
			{
				fieldCalibrationState = S_error;
			}
		}
		checkAbort(1);

		break;

	case S_confirm90:
		displayCalibrationPressure(90.0);

		BUTTON_read_IO();
		checkAbort(1);
		if (current_IO )
		{
			calibrationPauseTimer = 0;
			calibrationFlashTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;

//				calibrationTorque = pointlist[4].torque;
				calibrationTorque = calPointList[0].torque * 4.5;

				BUTTON_timed_read_IO_init();
				fieldCalibrationState = S_torque90;

			}
		}
		break;

	case S_torque90:
		displayAdjustTorque();

		if ( button_io_status = BUTTON_timed_read_IO() )
		{
			if ( button_io_status == 1 )
			{
				CalibrateCollectAverage();
			}
			else
			{
				CalibrateFinishAverage(4,90.0);

				calibrationPauseTimer = 0;
				calibrationFlashTimer = 0;
				fieldCalibrationState = S_confirmTorque90;
			}
		}
		checkAbort(0);

		break;

	case S_confirmTorque90:
		displayCalibrationTorque(calibrationTorque);

		BUTTON_read_IO();
		checkAbort(0);

		if (current_IO )
		{
			calibrationFlashTimer = 0;
			calibrationPauseTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationFlashTimer = 0;
				calibrationPauseTimer = 0;
				if ( calibrationSanityCheck() )
				{
					fieldCalibrationState = S_saveQuery;
				}
				else
				{
					fieldCalibrationState = S_error;
				}
			}
		}

		break;

	case S_saveQuery:

		calibrationFlashTimer++;

		if ( calibrationFlashTimer < 16 )
			LCD_save();
		else if ( calibrationFlashTimer > 31)
			calibrationFlashTimer = 0;

		if ( BUTTON_read_IO())
		{
			unsigned int sensorIndex;

			calibrationPauseTimer = 0;
			calibrationFlashTimer = 0;
			fieldCalibrationState = S_confirming;

			for ( sensorIndex = 0 ; sensorIndex < NUM_CAL_POINTS ; sensorIndex++ )
			{
				EEPROM_writeWord16( sensorIndex*3+0,calPointList[sensorIndex].count);
				EEPROM_writeFloat(sensorIndex*3+1,calPointList[sensorIndex].pressure);
				EEPROM_writeFloat(sensorIndex*3+2,calPointList[sensorIndex].torque);
			}

			loadPoints();				// reload new points
		}
		else
		{
			checkAbort(1);
		}

		break;

	case S_confirming:
		LCD_save();

		if ( BUTTON_read_IO())
		{
			calibrationPauseTimer = 0;
			calibrationFlashTimer = 0;
		}
		else
		{
			calibrationPauseTimer++;
			if ( calibrationPauseTimer > TWO_SECONDS )
			{
				calibrationPauseTimer = 0;
				calibrationFlashTimer = 0;
				fieldCalibrationState = S_calOff;
				power_state = P_OFF;
			}
		}

		break;

	case S_error:
		LCD_error();
		calibrationPauseTimer++;
		if ( calibrationPauseTimer > TEN_SECONDS )
		{
			calibrationPauseTimer = 0;
			calibrationFlashTimer = 0;
			fieldCalibrationState = S_calOff;
			power_state = P_OFF;
		}
		break;
	}

}


