/*
 * parser.c
 *
 *  Created on: Oct 23, 2013
 *      Author: mark
 *
 *  Added support for LTC3335 buck/boost w/coulomb counter
 *  via conditional compilations and new function calls
 *
 *      Date: 30 June, 2022
 *          author: mw
 */


#include <stdio.h>

#include "hwdef.h"
#include "eeprom.h"
#include "uart.h"
#include "lcd.h"
#include "parser.h"
#include "states.h"
#include "calibrate.h"

#ifndef __LTC3335
extern float fvbatt;
#else
extern unsigned char CoulombCount[4];
#endif

unsigned int sensorIndex;
int sensorRaw;
float sensorPressure;
float sensorTorque;

extern unsigned long serialNumber;
int SerialEnable = 0;

int CalibrateEnable = 0;
int ActuationEnable = 0;
int MaxTorqueEnable = 0;
unsigned int password = 0;
unsigned int temp;

unsigned int rxIndex = 0;
char rxBuff[RX_BUFF_SIZE];

extern int auto_off_timer_ticks;
extern long calibration_fds;
extern long unsigned actuationCount;
extern int Max_Torque;

int	calRow;
float calPressure;
float calTorque;
unsigned int calADCcounts;
extern int calibrationTimeout;
extern unsigned char RefreshPeriod;
unsigned int tmpvalue;


int AdjustCount(unsigned int indexLow,float targetPressure);
float AdjustTorque(unsigned int indexLow,float targetPressure);


void CmdParse(void)
{
		if ( rxIndex )
		{
			unsigned char ch;
			unsigned char buff[6];

			auto_off_timer_ticks = 0;
			calibrationTimeout = 0;
			password = 0;

			ch = rxBuff[rxIndex -1];

			switch ( ch )
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '.':
			case '-':
			case ',':
			case ':':
			case ' ':
			case '\t':
				break;

//	case 0x07:	// backspace
//	case 0x7f:
//		if ( rxIndex > 0 )
//			rxIndex--;
//		rxBuff[rxIndex] = 0;
//		break;

			case 'p':
				convertToAsciiDec(buff,calibration_fds);
				UART_Transmit( buff[0] );
				UART_Transmit( buff[1] );
				UART_Transmit( buff[2] );
				UART_Transmit( buff[3] );
				UART_Transmit( buff[4] );
				UART_Transmit( buff[5] );
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);
				rxIndex = 0;
				rxBuff[0] = 0;
				break;
			case 'v':
#ifndef __LTC3335
                LCD_convertToAsciiFloat3D(buff,fvbatt);
                UART_Transmit( buff[0] );
                UART_Transmit( buff[1] );
                UART_Transmit( buff[2] );
                UART_Transmit( buff[3] );
                UART_Transmit( buff[4] );
                UART_Transmit( buff[5] );
#else
                UART_Transmit( CoulombCount[0] );
#endif
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);
				rxIndex = 0;
				rxBuff[0] = 0;
				break;
#if defined(TEST2)
			case 'u':
			case 'U':
				UART_Transmit('U');		// baud rate measurement check
				UART_Transmit('U');
				UART_Transmit('U');
				UART_Transmit('U');
				UART_Transmit('U');
				UART_Transmit('U');
				UART_Transmit('U');
				UART_Transmit('U');
				UART_Transmit('U');
				UART_Transmit('U');

				rxIndex = 0;
				rxBuff[0] = 0;
				break;
#endif

			case 'r':
				sscanf(rxBuff,"%d,%d,%f,%f",&sensorIndex,&sensorRaw,&sensorPressure,&sensorTorque);

				if ( CalibrateEnable )
				{
					EEPROM_writeWord16( sensorIndex*3+0,sensorRaw);
					EEPROM_writeFloat(sensorIndex*3+1,sensorPressure);
					EEPROM_writeFloat(sensorIndex*3+2,sensorTorque);

					sprintf(rxBuff,"%d,%d,%f,%f",sensorIndex,sensorRaw,sensorPressure,sensorTorque);
					rxIndex = 0;

					ch = rxBuff[rxIndex++];
					while ( ch != 0 )
					{
						UART_Transmit(ch);
						ch = rxBuff[rxIndex++];
					};
				}
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;

				break;

			case 'f':
				sscanf(rxBuff,"%d",&sensorIndex);
				sensorRaw = EEPROM_readWord16(EE_CALIBRATION+sensorIndex*3);
				sensorPressure = EEPROM_readFloat(EE_CALIBRATION+sensorIndex*3+1);
				sensorTorque   = EEPROM_readFloat(EE_CALIBRATION+sensorIndex*3+2);

				sprintf(rxBuff,"%d,%d,%f,%f",sensorIndex,sensorRaw,sensorPressure,sensorTorque);
				rxIndex = 0;

				ch = rxBuff[rxIndex++];
				while ( ch != 0 )
				{
					UART_Transmit(ch);
					ch = rxBuff[rxIndex++];
				};
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;

				break;

			case 'C':
				sscanf(rxBuff,"%ud",&password);
				if ( password == 10000 )
				{
					ClearCalibration();
					rxIndex = 0;
					rxBuff[0] = 0;
					UART_Transmit('X');
				}
				else if ( password == 22222 )
				{
					if ( ActuationEnable )
					{
						actuationCount = 0;
						rxIndex = 0;
						rxBuff[0] = 0;
						UART_Transmit('X');
					}
				}
				else if ( password == 0 )
				{
					sprintf(rxBuff,"%ld",actuationCount);
					rxIndex = 0;

					ch = rxBuff[rxIndex++];
					while ( ch != 0 )
					{
						UART_Transmit(ch);
						ch = rxBuff[rxIndex++];
					};
				}
				else
				{
					CalibrateEnable = FALSE;
					ActuationEnable = FALSE;
					MaxTorqueEnable = FALSE;
					password = 0;
				}
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;

				break;

			case 'M':
				sscanf(rxBuff,"%ud",&password);
				if ( password == 10000 )
				{
					CalibrateEnable = TRUE;
					UART_Transmit('X');
				}
				else if ( password == 22222 )
				{
					ActuationEnable = TRUE;
					UART_Transmit('X');

				}
				else if ( password == 33333 )
				{
					MaxTorqueEnable = TRUE;
					UART_Transmit('X');

				}
#if defined(TEST1)
				else if ( password == 13570 )
				{
					calPointList[0].count = 5767;
					calPointList[0].pressure = 20.0;
					calPointList[0].torque = 400.0;

					calPointList[1].count = 11534;
					calPointList[1].pressure = 40.0;
					calPointList[1].torque = 800.0;

					calPointList[2].count = 17302;
					calPointList[2].pressure = 60.0;
					calPointList[2].torque = 1200.0;

					calPointList[3].count = 23069;
					calPointList[3].pressure = 80.0;
					calPointList[3].torque = 1600.0;

					calPointList[4].count = 28836;
					calPointList[4].pressure = 100.0;
					calPointList[4].torque = 2000.0;

					for ( sensorIndex = 0 ; sensorIndex < NUM_CAL_POINTS ; sensorIndex++ )
					{
						EEPROM_writeWord16( sensorIndex*3+0,calPointList[sensorIndex].count);
						EEPROM_writeFloat(sensorIndex*3+1,calPointList[sensorIndex].pressure);
						EEPROM_writeFloat(sensorIndex*3+2,calPointList[sensorIndex].torque);
					}

					loadPoints();				// reload new points
				}
#endif
				else if ( password == 13579 )
				{
					calPointList[0].count = 5000;
					calPointList[0].pressure = 20.0;
					calPointList[0].torque = 440.0;

					calPointList[1].count = 10000;
					calPointList[1].pressure = 40.0;
					calPointList[1].torque = 880.0;

					calPointList[2].count = 15000;
					calPointList[2].pressure = 60.0;
					calPointList[2].torque = 1320.0;

					calPointList[3].count = 20000;
					calPointList[3].pressure = 80.0;
					calPointList[3].torque = 1760.0;

					calPointList[4].count = 25000;
					calPointList[4].pressure = 100.0;
					calPointList[4].torque = 2200.0;

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
					CalibrateEnable = FALSE;
					ActuationEnable = FALSE;
					MaxTorqueEnable = FALSE;
					password = 0;
				}
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;

				break;

			case 'm':
				if ( CalibrateEnable )
				{
					for ( sensorIndex = 0 ; sensorIndex < NUM_CAL_POINTS ; sensorIndex++ )
					{
						EEPROM_writeWord16( sensorIndex*3+0,calPointList[sensorIndex].count);
						EEPROM_writeFloat(sensorIndex*3+1,calPointList[sensorIndex].pressure);
						EEPROM_writeFloat(sensorIndex*3+2,calPointList[sensorIndex].torque);
					}

					loadPoints();				// reload new points
					CalibrateEnable = FALSE;

					power_state = P_OFF;

					password = 0;
					rxIndex = 0;
					rxBuff[0] = 0;
					UART_Transmit('X');
					UART_Transmit(0x0D);
					UART_Transmit(0x0A);
				}


				break;

			case 'i':
				serialNumber = EEPROM_readLong32(EE_SERIAL_NUMBER);
				sprintf(rxBuff,"%lu",serialNumber);
				rxIndex = 0;

				ch = rxBuff[rxIndex++];
				while ( ch != 0 )
				{
					UART_Transmit(ch);
					ch = rxBuff[rxIndex++];
				};

				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;
				break;

			case 'u':
				sprintf(rxBuff,"%u",Max_Torque);
				rxIndex = 0;

				ch = rxBuff[rxIndex++];
				while ( ch != 0 )
				{
					UART_Transmit(ch);
					ch = rxBuff[rxIndex++];
				};

				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;
				break;

			case 'U':
				if ( MaxTorqueEnable )
				{
					if ( 1 == sscanf(rxBuff,"%uU",&temp))
					{
						if ( (temp >= 100 ) && ( temp <= 9000 ))
						{
							Max_Torque = temp;
							EEPROM_writeLong32(EE_MAX_TORQUE,Max_Torque);
							UART_Transmit('X');
						}
					}
				}
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				MaxTorqueEnable = FALSE;
				rxIndex = 0;
				rxBuff[0] = 0;
				break;

			case 'F':
				sscanf(rxBuff,"%ud",&password);
				if ( password == 10000)
				{
					SerialEnable = TRUE;
				}
				else
				{
					SerialEnable = FALSE;
					password = 0;
				}

				rxIndex = 0;
				rxBuff[0] = 0;
				UART_Transmit('X');
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);
				break;

			case 'I':
				if ( SerialEnable )
				{
					if ( 1 == sscanf(rxBuff,"%luI",&serialNumber))
					{
						EEPROM_writeLong32(EE_SERIAL_NUMBER,serialNumber);
						UART_Transmit('X');
					}
				}
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				SerialEnable = FALSE;
				rxIndex = 0;
				rxBuff[0] = 0;
				break;

			case 's':
#ifndef __LTC3335
                sprintf(rxBuff,"%ld ADC,%f PSI,%f FT-LBS,%f V",calibration_fds,ConvertFromCounts(calibration_fds,0,0),ConvertFromCounts(calibration_fds,1,0),fvbatt);
#else
                sprintf(rxBuff,"%ld ADC,%f PSI,%f FT-LBS,%d C",calibration_fds,ConvertFromCounts(calibration_fds,0,0),ConvertFromCounts(calibration_fds,1,0),CoulombCount[0]);
#endif
                rxIndex = 0;

				ch = rxBuff[rxIndex++];
				while ( ch != 0 )
				{
					UART_Transmit(ch);
					ch = rxBuff[rxIndex++];
				};

				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;

				break;

			case 'V':
				sprintf(rxBuff,"%4.2f",VERSION_ID);
				rxIndex = 0;

				ch = rxBuff[rxIndex++];
				while ( ch != 0 )
				{
					UART_Transmit(ch);
					ch = rxBuff[rxIndex++];
				};

				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;

				break;

			case 'S':
				sscanf(rxBuff,"%d",&calRow);
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;

				power_state = P_FACTORY_CALIBRATE;
				calibrationTimeout = 0;
				calADCcounts = calibration_fds;

				switch (calRow)
				{
				case 0:
					calPressure = 20.0;
					break;

				case 1:
					calPressure = 40.0;
					break;

				case 2:
					calPressure = 60.0;
					break;

				case 3:
					calPressure = 80.0;
					break;

				case 4:
					calPressure = 100.0;
					break;

				default:
					calRow = -1;
					calPressure = 0.0;
					break;

				}

				break;

			case 'z':
				sprintf(rxBuff,"%d",RefreshPeriod);
				rxIndex = 0;

				ch = rxBuff[rxIndex++];
				while ( ch != 0 )
				{
					UART_Transmit(ch);
					ch = rxBuff[rxIndex++];
				};

				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;
				break;

			case 'Z':
				sscanf(rxBuff,"%ud",&tmpvalue);
				RefreshPeriod = (unsigned char) tmpvalue;

				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				EEPROM_writeLong32(EE_REFRESH_TIME,RefreshPeriod);

				rxIndex = 0;
				rxBuff[0] = 0;
				break;

			case 'P':
				sscanf(rxBuff,"%f",&calPressure);
				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;
				break;

			case 'T':
				sscanf(rxBuff,"%f",&calTorque);

				if ( calRow >= 0 )
				{
					calRawPointList[calRow].count = calADCcounts;
					calRawPointList[calRow].pressure = calPressure;
					calRawPointList[calRow].torque = calTorque;
				}

				if ( calRow == 4 )
				{

#if defined(FORCE_CAL_TO_DEFAULT)
					calRawPointList[0].count = 5910;
					calRawPointList[0].pressure = 19.7;
					calRawPointList[0].torque = 39.4;

					calRawPointList[1].count = 12150;
					calRawPointList[1].pressure = 40.5;
					calRawPointList[1].torque = 81.0;

					calRawPointList[2].count = 18030;
					calRawPointList[2].pressure = 60.1;
					calRawPointList[2].torque = 120.2;

					calRawPointList[3].count = 23850;
					calRawPointList[3].pressure = 79.5;
					calRawPointList[3].torque = 159.0;

					calRawPointList[4].count = 30210;
					calRawPointList[4].pressure = 100.7;
					calRawPointList[4].torque = 201.4;
#endif

					calPointList[0].count = AdjustCount(0,20.0);
					calPointList[0].torque = AdjustTorque(0,20.0);
					calPointList[0].pressure = 20.0;

					calPointList[1].count = AdjustCount(1,40.0);
					calPointList[1].torque = AdjustTorque(1,40.0);
					calPointList[1].pressure = 40.0;

					calPointList[2].count = AdjustCount(2,60.0);
					calPointList[2].torque = AdjustTorque(2,60.0);
					calPointList[2].pressure = 60.0;

					calPointList[3].count = AdjustCount(3,80.0);
					calPointList[3].torque = AdjustTorque(3,80.0);
					calPointList[3].pressure = 80.0;

					calPointList[4].count = AdjustCount(3,100.0);
					calPointList[4].torque = AdjustTorque(3,100.0);
					calPointList[4].pressure = 100.0;
				}


				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;
				break;


			default:
				convertToAsciiHex(buff,ch);
				UART_Transmit( buff[2] );
				UART_Transmit( buff[3] );

				UART_Transmit(' ');

				UART_Transmit(0x0D);
				UART_Transmit(0x0A);

				rxIndex = 0;
				rxBuff[0] = 0;

			}
		}
//		else
//		{
//			if ( UART_TransmitReady() )
//			{
//				UART_Transmit('U');
//			}
//		}
}

static float slope;
static float offset;

int AdjustCount(unsigned int indexLow,float targetPressure)
{
	slope = (calRawPointList[indexLow+1].count - calRawPointList[indexLow].count)/(calRawPointList[indexLow+1].pressure - calRawPointList[indexLow].pressure );
	offset = calRawPointList[indexLow].count;
	return offset + slope*(targetPressure - calRawPointList[indexLow].pressure);
}

float AdjustTorque(unsigned int indexLow,float targetPressure)
{


	slope = (calRawPointList[indexLow+1].torque - calRawPointList[indexLow].torque)/(calRawPointList[indexLow+1].pressure - calRawPointList[indexLow].pressure );
	offset = calRawPointList[indexLow].torque;
	return offset + slope*(targetPressure - calRawPointList[indexLow].pressure);
}



