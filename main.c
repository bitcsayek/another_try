/*
 * main.c
 *  Created on: Sep 12, 2013
 *      Author: mark
 *
 *
 *  Added code to initialize and read LTC3335 Buck/Boost regulator w/I2C Coulomb counter
 *      Date: 30 June, 2022
 *      Author: mw
 */
#include <stdio.h>
#include <math.h>

#include "hwdef.h"
#include "timeconst.h"
#include "uart.h"
#include "lcd.h"
#include "battery.h"
#include "buttons.h"

#include "deltaSigma.h"
#include "states.h"
#include "eeprom.h"
#include "parser.h"
#include "calibrate.h"
#include "MaxSetup.h"
#include "FieldTorqueInc.h"
#include "FieldDisplayRefresh.h"
#include "I2C.h"


int tickClock;
//int holdTicks;
int freezeTicks;
extern int lowPower;

float steady_pressure;
float freeze_pressure;
float hold_pressure;
float current_pressure;

float display_pressure;
float display_torque;

float steady_delta_pressure;
//float release_pressure;
float delta_drop_pressure;


#define HIS_SIZE 4
unsigned int hi;
struct {
	int i;
	float f;
} history[HIS_SIZE];



#define PSI_ROUNDING 1.0
#define PSI_HYSTORSIS 0.5

//#define TORQUE_ROUNDING 5.0
//#define TORQUE_HYSTORSIS 2.5

int TorqueRounding;

unsigned char RefreshPeriod;
unsigned char RefreshCounter;


extern int calibrationTimeout;
extern int lastStatus ;

int auto_off_timer_ticks = 0;

#define MAX_SETUP_TICKS	(32*10)
int max_setup_ticks = 0;


long unsigned actuationCount;
enum {ACT_STEADY,ACT_TRIGGERED,ACT_FREERUN} actuationState = ACT_STEADY;
enum {HOLD_OFF,HOLD_FREEZE,HOLD_CHECKING,HOLD_BLINK/*,HOLD_STABLIZE*/} holdState = HOLD_OFF;

unsigned long serialNumber = 0;


#define AUTO_OFF_THRESHOLD 800

#define DISPLAY_HOLD_TICKS_PRESSED  4		// to reenable display unhold
#define DISPLAY_HOLD_TICKS_RELEASED 4		// to reenable display hold


#pragma vector = 40
__interrupt void RTC_ISR(void)
{
}
#pragma vector = 41
__interrupt void LCD_C_ISR(void)
{
}
#pragma vector = 42
__interrupt void TIMER3_A1_ISR(void)
{
}
#pragma vector = 43
__interrupt void TIMER4_A0_ISR(void)
{
}
#pragma vector = 44
__interrupt void PORT2_ISR(void)
{
	P2IFG = 0x00;	// clear any pending flags
    __bic_SR_register_on_exit(LPM3_bits); // Exit LPM0 on return
}
#pragma vector = 45
__interrupt void TIMER2_A1_ISR(void)
{
}
#pragma vector = 46
__interrupt void TIMER2_A0_ISR(void)
{
}
#pragma vector = 47
__interrupt void PORT1_ISR(void)
{
	P1IFG = 0x00;	// clear any pending flags
    __bic_SR_register_on_exit(LPM3_bits); // Exit LPM3 on return
}
#pragma vector = 48
__interrupt void TIMER1_A1_ISR(void)
{
}
#pragma vector = 49
__interrupt void TIMER1_A0_ISR(void)
{
}
#pragma vector = 50
__interrupt void DMA_ISR(void)
{
}
#pragma vector = 51
__interrupt void AUX_ISR(void)
{
}
#pragma vector = 52
__interrupt void USCI_A2_ISR(void)
{
	UART_Receive();
}
#pragma vector = 53
__interrupt void USCI_A1_ISR(void)
{
}
#pragma vector = 54
__interrupt void TIMER0_A1_ISR(void)
{
    switch (__even_in_range(TA0IV, 14))
    {
        case  TA0IV_NONE: break;             // No interrupt
        case  TA0IV_TA0CCR1: break;          // TA0CCR1_CCIFG
        case  TA0IV_TA0CCR2: break;          // TA0CCR2_CCIFG
        case  6: break;                      // Reserved
        case  8: break;                      // Reserved
        case 10: break;                      // Reserved
        case 12: break;                      // Reserved
        case TA0IV_TA0IFG:                   // TA0IFG
        	tickClock++;
			TA0CTL &= ~TAIFG;			     // clear any pending timer interrupt.
			if ( !lowPower )
			{
				__bic_SR_register_on_exit(LPM3_bits); // Exit LPM3 on return
			}
			lowPower = 0;
            break;
        default: break;
    }
}
#pragma vector = 55
__interrupt void TIMER0_A0_ISR(void)
{
    switch (__even_in_range(TA0IV, 14))
    {
        case  TA0IV_NONE: break;             // No interrupt
        case  TA0IV_TA0CCR1: break;          // TA0CCR1_CCIFG
        case  TA0IV_TA0CCR2: break;          // TA0CCR2_CCIFG
        case  6: break;                      // Reserved
        case  8: break;                      // Reserved
        case 10: break;                      // Reserved
        case 12: break;                      // Reserved
        case TA0IV_TA0IFG:                   // TA0IFG
        	tickClock++;
            __bic_SR_register_on_exit(LPM3_bits); // Exit LPM0 on return
            break;
        default: break;
    }
}
#pragma vector = 56
__interrupt void SD24B_ISR(void)
{
}
#pragma vector = 57
__interrupt void ADC10_ISR(void)
{
}
#pragma vector = 58
__interrupt void USCI_B0_ISR(void)
{
}
#pragma vector = 59
__interrupt void USCI_A0_ISR(void)
{
}
#pragma vector = 60
__interrupt void WDT_ISR(void)
{
}
#pragma vector = 61
__interrupt void UNMI_ISR(void)
{
}
#pragma vector = 62
__interrupt void SYSNMI_ISR(void)
{
}

void to_lpm(void)
{
	TA0CTL &= ~TAIFG;						// clear any pending timer interrupt.
	TA0CTL |= TAIE;							// enable interrupts
#ifndef __LTC3335
	if ( ADPTR_ON_PORT & ADPTR_ON_BIT)
	{
		UCA2IE |= UCRXIE;

		__bis_SR_register(LPM0_bits | GIE);     // Enter LPM1, enable interrupts , leave smclk running for the uart
		__no_operation();                       // For debugger
	}
	else
#endif
	{
		UCA2IE = 0;
		__bis_SR_register(LPM3_bits | GIE);     // Enter LPM1, enable interrupts , smclk off for lower power
		__no_operation();                       // For debugger
	}

	TA0CTL &= ~TAIE;						// disable interrupts
	TA0CTL &= ~TAIFG;						// clear any pending timer interrupt.
}



long ds;
long fds;
//int display_fds;
long freeze_fds;
long calibration_fds;
long steady_state_fds;
long last_display_fds;
//long release_fds;
long hold_fds;
int battery_timer = 0;

#ifndef __LTC3335
int vbatt;
float fvbatt;
#endif


#define IIR 1
//#define IIR_RELEASE 16
#define IIR_CAL 16
#define IIR_STEADY  256
#define IIR_STEADY_RISE 4
#define IIR_FREEZE 16

void displayCalibrationPressure(float targetPressure);
void displayCalibrationTorque(float targetTorque);
void FieldCalibrationStateMachine(void);


float pressure;
float torque;
int Max_Torque;

#define DISPLAY_PAUSE 32

void DisplayVersion(void)
{
	LCD_clear(DISPLAY_ALL_ON);
	LCD_paint();

	tickClock = 0;
	while ( tickClock < 16)
	{
		to_lpm();
	}

	LCD_clear(DISPLAY_ALL_OFF);
	LCD_displayFloat2D(VERSION_ID);

	LCD_nm(1);
	LCD_ftlbs(1);
	LCD_psi(1);
	LCD_bar(1);
	LCD_kPa(1);

	LCD_paint();

	tickClock = 0;
	while ( tickClock < DISPLAY_PAUSE)
	{
		Delta_ReadFinish();		// call this while waiting to keep the converter primed
		to_lpm();
	}

	tickClock = 0;

//	LCD_clear(DISPLAY_ALL_ON);
//	while ( tickClock < 16)
//	{
//		to_lpm();
//	}
//	tickClock = 0;
}

void DisplayMaxTorque(void)
{
	tickClock = 0;
	LCD_clear(DISPLAY_ALL_OFF);
	LCD_paint();

	tickClock = 0;
	while ( tickClock < 8)
	{
		Delta_ReadFinish();		// call this while waiting to keep the converter primed
		to_lpm();
	}

	LCD_ftlbs(1);
	LCD_displayInt(Max_Torque);
	LCD_paint();

	tickClock = 0;
	while ( tickClock < DISPLAY_PAUSE)
	{
		to_lpm();
	}

	LCD_clear(DISPLAY_ALL_OFF);
	LCD_paint();

	tickClock = 0;
	while ( tickClock < 8)
	{
		Delta_ReadFinish();		// call this while waiting to keep the converter primed
		to_lpm();
	}

	tickClock = 0;
}

void DisplayActuations(void)
{
	LCD_clear(DISPLAY_ALL_OFF);
	LCD_displayFloat0D(actuationCount/10);
	LCD_paint();

//	TA0CCR0 = 0x03FF;		// 1/32 second

//	TA0CTL &= ~TAIFG;						// clear any pending timer interrupt.
//	TA0CTL |= TAIE;							// enable interrupts

	tickClock = 0;
	while ( tickClock < DISPLAY_PAUSE )
	{
		Delta_ReadFinish();		// call this while waiting to keep the converter primed
		to_lpm();
	}
	LCD_clear(DISPLAY_ALL_OFF);
	LCD_displayX10();
	LCD_paint();

	tickClock = 0;
	while ( tickClock < DISPLAY_PAUSE )
	{
		Delta_ReadFinish();		// call this while waiting to keep the converter primed
		to_lpm();
	}
}

void DisplayStartup(void)
{
	DisplayVersion();
	DisplayMaxTorque();
	DisplayActuations();

}


void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT
	WDTCTL = WDTPW + WDTHOLD;   // Stop WDT
	UCSCTL3 |= SELREF_2;
    UCSCTL4 |= SELA_2;

	P1SEL = 0x84;
	P2SEL = 0x0F;
	P3SEL = 0x00;
	P4SEL = 0xF0;
	P5SEL = 0xFF;
	P6SEL = 0xFF;
	PDSEL = 0x00;
	PESEL = 0x00;
	PJSEL = 0x0f;	// clocks selected

	// direction:  1 is output, 0 is input

	P1DIR = 0xBB;
	P2DIR = 0x0A;
	P3DIR = 0xFF;
	P4DIR = 0x0F;
	P5DIR = 0x00;
	P6DIR = 0x00;
//	PJDIR = 0x0A;
	PJDIR = 0x0F;	// clocks out

	P1OUT = 0xff;
	P2OUT = 0xff;
	P3OUT = 0xff;
	P4OUT = 0xff;
	P5OUT = 0xff;
	P6OUT = 0xff;

	PDOUT = 0xff;
	PEOUT = 0xff;
	PJOUT = 0xff;	//

	SEN_EN_PORT &= ~SEN_EN_BIT; //enable the pressure transducer

	//* not used with LTC3335
#ifndef __LTC3335
	BAT_TS_EN_PORT &= ~BAT_TS_EN_BIT;
#endif



	// the normal timer keeping interrupt
	TA0CCR0 = 0x03FF;		// 1/32 second

	TA0CTL = TASSEL__ACLK | MC_1 | TAIE | TACLR;	// ACLK, count up, interrupts enabled

//	to_lpm();

	LCD_init();

	UART_init();

	EEPROM_init();

    power_state = P_ON;

#ifdef __LTC3335
	{
        //this code only executes once, at power up
        //if any errors are detected at this time something is very wrong at the board level

	    ConfigCounter();   //set up the LTC3335 coulomb counter registers
#if 0
         if(Alarm[0] > 0){
	        //LTC3335 has thrown an alarm
	        if(Alarm == INDUCTOR_FAULT){
	            //most likely cause at this stage is an inductor short
	            //put error message on display - Err1
	        }else if(Alarm == COUNTER_FAULT){
	            //most likely cause for this fault at startup is either a communication error
	            //or an internal part fault
	            //put error message on display - Err2
	        }else if(Alarm == PRESET_FAULT){
	            //this fault should not occur at power up as the internal counter is automatically cleared
	            //most likely cause is either a communication fault or an internal fault
	            power_state = P_VERY_LOW_BATTERY;
	        }
	        ClearAlarm();
	    }
#endif
	}

	//TestCounter(0);
    //for(;;);
#endif

//	power_state = P_ON;

	display = D_TORQUE;
	pressure_units = D_PSI;
	torque_units = D_FTLBS;


	loadPoints();

	// verify that we have a good serial number
	serialNumber = EEPROM_readLong32(EE_SERIAL_NUMBER);
	if ( serialNumber == 0xffffffff )
	{
		serialNumber = 0x0000;
		EEPROM_writeLong32(EE_SERIAL_NUMBER,serialNumber);
	}

	actuationCount = EEPROM_readLong32(EE_ACTUATION_COUNT);
	if ( actuationCount == 0xffffffff )
	{
		actuationCount = 0x0;
	}

	Max_Torque = EEPROM_readLong32(EE_MAX_TORQUE);
	if ( Max_Torque == 0xffffffff )
	{
		Max_Torque = 2200;
	}

	TorqueRounding = EEPROM_readLong32(EE_TORQUE_ROUNDING);
	if ( ( TorqueRounding == 0xffffffff ) || TorqueRounding == 0 )
	{
		TorqueRounding = 5;
	}

	RefreshPeriod = EEPROM_readLong32(EE_REFRESH_TIME);
	if ( ( RefreshPeriod == 0xffffffff ) || ( RefreshPeriod == 0 ) )
	{
		RefreshPeriod = 1;
		EEPROM_writeLong32(EE_REFRESH_TIME,RefreshPeriod);
	}

	float tmp_version = EEPROM_readFloat(EE_LAST_VERSION);
	float tmp_version_f = VERSION_ID;
	if ( tmp_version_f != tmp_version )
	{
		RefreshPeriod = 1;
		EEPROM_writeFloat(EE_LAST_VERSION,VERSION_ID);
		EEPROM_writeLong32(EE_REFRESH_TIME,RefreshPeriod);
	}



#ifndef __LTC3335
    vbatt = BATTERY_check();
	fvbatt = (float)vbatt/1023.0*2.5*2.0;
#else
    ReadCoulombs();                //get the coulomb count
#endif

	max_setup_ticks = MAX_SETUP_TICKS;
	DisplayStartup();

	// seed the irr filters
	to_lpm();
	fds  = Delta_ReadFinish();

	calibration_fds = fds;
	steady_state_fds = fds;
//	release_fds = fds;

//	holdTicks = 0;
	tickClock = 0;

	while ( 1 )
	{
		// wait for timer tick of 1/32 second
		to_lpm();

		if ( power_state == P_OFF)
		{
			BUTTON_wait_for_on();
			{
				if ( power_state == P_ON )
				{
					max_setup_ticks = MAX_SETUP_TICKS;
					DisplayStartup();
				}
			}
		}

		RefreshCounter++;
		if ( RefreshCounter > RefreshPeriod)
		{
			RefreshCounter = 1;
		}

		LCD_clear(DISPLAY_ALL_OFF);
		LCD_batFrm(1);

		if ( ( power_state == P_ON ) || ( power_state == P_FACTORY_CALIBRATE ) || ( power_state == P_FIELD_CALIBRATE ) )
		{
			ds  = Delta_ReadFinish();

			fds = ( IIR * fds + ds )/(1+IIR);

			calibration_fds = (IIR_CAL * calibration_fds + ds)/(1+IIR_CAL);

			freeze_fds = ( IIR_FREEZE * freeze_fds + ds)/(1+IIR_FREEZE);

			if ( fds > steady_state_fds)
				steady_state_fds = (IIR_STEADY_RISE * steady_state_fds + fds ) / (1+IIR_STEADY_RISE);
			else
				steady_state_fds = (IIR_STEADY * steady_state_fds + fds ) / (1+IIR_STEADY);

//			release_fds = ( IIR_RELEASE * release_fds + fds)/(1+IIR_RELEASE);

		}

		// check for the set max torque change
		if ( power_state == P_ON )
		{
			if ( current_IO == 0 )
			{
				max_setup_ticks = 0;
			}
			if ( max_setup_ticks )
			{
				LCD_ftlbs(1);
				LCD_displayInt(Max_Torque);
//				LCD_paint();
				max_setup_ticks--;
				if ( max_setup_ticks == 0 )
				{
					initMaxSetupStateMachine(Max_Torque);
					power_state = P_MAX_SETUP;

				}
			}
		}


		if ( (tickClock & 0x1f) == 0 )	// check every 1 second(s)
		{

#ifndef __LTC3335
		    vbatt = (vbatt*3 + BATTERY_check())/4;   // small filter(T=4sec)

            if ( power_state == P_ON )
			{
				fvbatt = (fvbatt * 3.0 + (float)vbatt/1023.0*2.5*2.0)/4.0; // small filter (T=4 sec)
			}
#else
            ReadCoulombs();                            //get coulomb count
#endif
		}

//		LCD_clear(DISPLAY_ALL_OFF);
//		LCD_batFrm(1);


#ifndef __LTC3335
		if ( ADPTR_ON_PORT & ADPTR_ON_BIT)
		{
 			if ( !(CHARGING_B_PORT & CHARGING_B_BIT) )
			{
				battery_timer++;

				// each tick is 1/32 of a second, so test accordingly

				if ( battery_timer > 63)
					battery_timer = 0;

				if ( battery_timer > 47)
				{
					LCD_batHi(1);
				}
				if ( battery_timer > 31)
				{
					LCD_batMid(1);
				}
				if ( battery_timer > 15)
				{
					LCD_batLow(1);
				}
			}
			else
			{
				LCD_batHi(1);
				LCD_batMid(1);
				LCD_batLow(1);
			}
		}
		else
		    //set the number of bars according to the battery voltage
		{
		    if ( vbatt > VT_1 )
			{
				LCD_batHi(1);
			}
			if ( vbatt > VT_2 )
			{
				LCD_batMid(1);
			}
			if ( vbatt > VT_3 )
			{
				LCD_batLow(1);
			}
			if ( vbatt > VT_4 )
			{
				// just the frame.
			}
            else
            {
                // blink the frame, should disable as well.
                //power_state = P_VERY_LOW_BATTERY; // this has been commnted as Patrik wants the torque to appear while the battery is blinking
                power_state = P_ON; // this has been added as Patrik wants the torque to appear while the battery is blinking
                battery_timer++;
                if ( battery_timer > 31)
                    battery_timer = 0;
                if ( battery_timer > 15)
                {
                   // LCD_batFrm(0);
                }
            }
        }
#else
        //set the number of bars according to the battery coulomb level
		{

            if (CoulombCount[0] <= CL_CNT_1 ){                                  //less than 1035 coulombs burned
                LCD_batHi(1);                                                   //fully charged battery
                LCD_batMid(1);
                LCD_batLow(1);
            }
            if (CoulombCount[0] > CL_CNT_1 && CoulombCount[0] <= CL_CNT_2){     //more than 1035 but less than 2070 coulombs burned
                LCD_batMid(1);                                                  //2 bars
                LCD_batLow(1);
            }
            if (CoulombCount[0] > CL_CNT_2 && CoulombCount[0] <= CL_CNT_3){     //more than 2070 but less than 3105 coulombs burned
                LCD_batLow(1);                                                  //1 bar
            }
            if (CoulombCount[0] > CL_CNT_3  && CoulombCount[0] <= CL_CNT_4){    //more than 3105 but less than 3726 coulombs burned
                                                                                //just the frame 0 bars
            }
            if(CoulombCount[0] > CL_CNT_4)                                      //dead battery level - more than 3726
            {
                // blink the frame, should disable as well.
                power_state = P_VERY_LOW_BATTERY;
                battery_timer++;
                if ( battery_timer > 31){
                    battery_timer = 0;
                }
                if ( battery_timer > 15){
                    LCD_batFrm(0);
                }
            }
		}
#endif

//		if ( power_state == P_OFF ) 	// shouldn't happen except after a calibration timeout or abort
//		{
//			continue;
//		}



		if ( (( power_state == P_VERY_LOW_BATTERY ) || ( power_state == P_ON)) && BUTTON_read_IO() )
		{
			power_state = P_OFF;
		    //power_state = P_ON;
			if ( BUTTON_wait_for_on())
			{
				if ( power_state == P_ON )
				{
					max_setup_ticks = MAX_SETUP_TICKS;
					DisplayStartup();
				}
			}

//			power_state = P_ON;
			auto_off_timer_ticks = 0;

			// seed the irr filters
			fds  = Delta_ReadFinish();
			freeze_fds = fds;
			calibration_fds = fds;
			steady_state_fds = fds;
//			release_fds = fds;

//			holdTicks = 0;

			actuationState = ACT_STEADY;
			holdState = HOLD_OFF;

			display = D_TORQUE;


#ifndef __LTC3335
            vbatt = BATTERY_check();
            fvbatt = (float)vbatt/1023.0*2.5*2.0;
#else
            ReadCoulombs();                //get the coulomb count
#endif
		}


		current_pressure = ConvertFromCounts(fds,0,1);
		steady_pressure  = ConvertFromCounts(steady_state_fds,0,1);
		freeze_pressure  = ConvertFromCounts(freeze_fds,0,1);


//		release_pressure = ConvertFromCounts(release_fds,0,1);

		steady_delta_pressure = steady_pressure - current_pressure;

		for ( hi = 0 ; hi < HIS_SIZE-1 ; hi++)
		{
			history[hi].i = history[hi+1].i;
			history[hi].f  = history[hi+1].f;
		}
		history[hi].i = fds;
		history[hi].f = current_pressure;


		// small pressure changes let the timeout timer advance for auto off.
		if (( fabs(steady_delta_pressure) < 0.5 ) || ( power_state != P_ON  ))
		{
			if ( auto_off_timer_ticks < AUTO_OFF_TICKS)
			{
				auto_off_timer_ticks++;
			}
			else
			{
				power_state = P_OFF;
				BUTTON_wait_for_on();
				{
					if ( power_state == P_ON )
					{
						max_setup_ticks = MAX_SETUP_TICKS;
						DisplayStartup();
					}
				}

				auto_off_timer_ticks = 0;

				// seed the irr filters
				fds  = Delta_ReadFinish();
				freeze_fds = fds;
				calibration_fds = fds;
				steady_state_fds = fds;
//				release_fds = fds;

//				holdTicks = 0;

				actuationState = ACT_STEADY;
				holdState = HOLD_OFF;

				display = D_TORQUE;


#ifndef __LTC3335
                vbatt = BATTERY_check();
				fvbatt = (float)vbatt/1023.0*2.5*2.0;
#else
                ReadCoulombs();
#endif
			}
		}
		else	// big changes reset the auto-off timer.
		{
			auto_off_timer_ticks = 0;
			calibrationTimeout = 0;
		}

#define SLOPE_ACTUATION_DROP 0.9264	// actuation loaded pressure drop    - from table and spread sheet
#define SLOPE_ACTUATION_FREE 0.7441	// actuation free run pressure drop  - from table and spread sheet
#define SLOPE_ACTUATION_RISE 0.9500 // return pressure rise              - from table and spread sheet

		// if we get a noticable drop, count it.
		if ( ( actuationState == ACT_STEADY ) && ( current_pressure < SLOPE_ACTUATION_DROP*steady_pressure) )
		{
			actuationState = ACT_TRIGGERED;
			actuationCount++;
		}

		// if the noticable drop was too big, uncount it.
		// because it  was a unloaded trigger, not to be counted.
		if ( ( actuationState == ACT_TRIGGERED ) && ( current_pressure < SLOPE_ACTUATION_FREE*steady_pressure ) )
		{
			actuationState = ACT_FREERUN;
			if ( actuationCount > 0 )
				actuationCount--;
		}

		// once the pressure starts climbing back, reset the the trigger.
		if ( ( actuationState != ACT_STEADY ) && ( current_pressure > SLOPE_ACTUATION_RISE*steady_pressure) )
		{
			if ( actuationState == ACT_TRIGGERED )
			{
				EEPROM_writeLong32(EE_ACTUATION_COUNT,actuationCount);
			}
			actuationState = ACT_STEADY;
		}

		delta_drop_pressure = history[HIS_SIZE-2].f - history[HIS_SIZE-1].f;

#define SLOPE_SLOW_DROP 0.0275	// manual pressure adjust threashold // was 0.025
#define SLOPE_FAST_DROP 0.9264	// trigger pull drop    - from table and spread sheet
#define SLOPE_RECOVERY  0.9632  // trigger release rise - from table and spread sheet

		// if we get a noticable drop, lock the display
		// avoid the freeze caused by the regulator bounce with holdTicks.
		if ( ( holdState == HOLD_OFF ) /*&& ( holdTicks == 0 )*/ && ( delta_drop_pressure > SLOPE_SLOW_DROP*steady_pressure ) &&  ( current_pressure < SLOPE_FAST_DROP*steady_pressure) && ( current_pressure > 0.0 ) )
		{
			holdState = HOLD_FREEZE;
			hold_fds = history[0].i;
			hold_pressure = history[0].f;
//			holdTicks = DISPLAY_HOLD_TICKS_PRESSED;
			freezeTicks = HOLD_FREEZE_RELEASE_TICKS;
		}

		if ( holdState == HOLD_FREEZE )
		{
//			BUTTON_read_PT_UP();
			if ( current_PT  /*|| current_EM */)
			{
				if ( 0 == freezeTicks-- )
				{
					holdState = HOLD_BLINK;
					freezeTicks = QUARTER_SECOND;
				}
			}
			else
			{
				freezeTicks = HOLD_FREEZE_RELEASE_TICKS;
			}
		}

		if (( holdState == HOLD_BLINK ) /*&& ( actuationState == ACT_STEADY )*/)
		{
			if ( 0 == freezeTicks-- )
			{
				holdState = HOLD_CHECKING;
			}
		}

		if (( holdState == HOLD_CHECKING ) /*&& ( actuationState == ACT_STEADY )*/)
		{
			if ( fabs(hold_pressure - freeze_pressure )/hold_pressure > 0.10 )
			{
//				holdState = HOLD_STABLIZE;	// ver 2.70
//				freezeTicks = TWO_SECONDS;
				holdState = HOLD_OFF;	// ver 2.71
			}
		}
		if ( power_state == P_VERY_LOW_BATTERY )
		{
#ifndef __LTC3335
			//after the charger brings it up a ways we can start running
			if ( vbatt >  VT_4_hyst )
			{
				power_state = P_ON;
				auto_off_timer_ticks = 0;
			}
#endif
			LCD_paint();
			continue;
		}
		if ( ( power_state == P_VERY_LOW_BATTERY ) || ( power_state == P_ON) && ( !max_setup_ticks ))
		{
			switch  ( display )
			{
			case D_PRESSURE:
				if ( BUTTON_read_PT_UP())
				{
					display = D_TORQUE;
					break;
				}
				switch ( pressure_units )
				{
				case D_PSI:
					if ( BUTTON_read_EM_DOWN())
					{
						pressure_units = D_BAR;
						break;
					}

					LCD_psi(1);

					if ( abs(fds - last_display_fds) > 3)
					{
						last_display_fds = fds;
					}

					if ( holdState == HOLD_OFF )
					{
						pressure = ConvertFromCounts(last_display_fds,0,1);
					}
					else
					{
						pressure = ConvertFromCounts(hold_fds,0,1);
					}

					if ( fabs(pressure - display_pressure) > PSI_HYSTORSIS)
					{
						display_pressure = pressure;
					}


					LCD_displayFloat1D(roundf(display_pressure));

					break;

				case D_BAR:
					if ( BUTTON_read_EM_DOWN())
					{
						pressure_units = D_KPA;

						break;
					}
					LCD_bar(1);

					if ( abs(fds - last_display_fds) > 3)
					{
						last_display_fds = fds;
					}


					if ( holdState == HOLD_OFF )
					{
						pressure = ConvertFromCounts(last_display_fds,0,1);
					}
					else
					{
						pressure = ConvertFromCounts(hold_fds,0,1);
					}

					if ( fabs(pressure - display_pressure) > PSI_HYSTORSIS)
					{
						display_pressure = pressure;
					}



//					display_pressure *= PSI2BAR;

					LCD_displayFloat3D(roundf(display_pressure)*PSI2BAR);

					break;

				case D_KPA:
					if ( BUTTON_read_EM_DOWN())
					{
						pressure_units = D_PSI;
						break;
					}
					LCD_kPa(1);

					if ( abs(fds - last_display_fds) > 3)
					{
						last_display_fds = fds;
					}


					if ( holdState == HOLD_OFF )
					{
						pressure = ConvertFromCounts(last_display_fds,0,1);
					}
					else
					{
						pressure = ConvertFromCounts(hold_fds,0,1);
					}


					if ( fabs(pressure - display_pressure) > PSI_HYSTORSIS)
					{
						display_pressure = pressure;
					}

//					display_pressure *= PSI2KPA;

					LCD_displayFloat1D(roundf(display_pressure)*PSI2KPA);

					break;
				}
				break;

			case D_TORQUE:
				if ( BUTTON_read_PT_UP())
				{
#if defined(NO_PRESSURE_DISPLAY)
					display = D_TORQUE;
#else
					display = D_PRESSURE;
					break;
#endif

				}
				switch ( torque_units )
				{
				case D_NM:

					if ( BUTTON_read_EM_DOWN())
					{
						torque_units = D_FTLBS;
						break;
					}
					LCD_nm(1);

					if ( abs(fds - last_display_fds) > 2)
					{
						last_display_fds = fds;
					}

					if ( holdState == HOLD_OFF )
					{
						torque = ConvertFromCounts(last_display_fds,1,1);
					}
					else
					{
						torque = ConvertFromCounts(hold_fds,1,1);
					}

					if ( fabs(torque - display_torque) > 0.75*(float)TorqueRounding )
					{
						if (RefreshCounter == RefreshPeriod )
						{
							display_torque = round(torque / (float)TorqueRounding)*(float)TorqueRounding;
						}
						LCD_displayFloat1D(display_torque*FTLB2NM);
					}
					if ( (int)display_torque > Max_Torque )
					{
						LCD_dashes();
					}
					else if (( holdState !=  HOLD_OFF ) && ( (int)current_pressure < 5) )
					{
						holdState = HOLD_OFF;
						LCD_dashes();
					}
					else if (( holdState !=  HOLD_OFF ) && ( (int)hold_pressure < 20) )
					{
						LCD_dashes();
					}
					else if (( holdState ==  HOLD_OFF ) && ( (int)current_pressure < 20 ))
					{
						LCD_dashes();
					}
					else
					{
						LCD_displayFloat1D(display_torque*FTLB2NM);
					}


					break;

				case D_FTLBS:
					if ( BUTTON_read_EM_DOWN())
						{
							torque_units = D_NM;
							break;
						}
					LCD_ftlbs(1);

					if ( abs(fds - last_display_fds) > 2)
					{
						last_display_fds = fds;
					}

					if ( holdState == HOLD_OFF )
					{
						torque = ConvertFromCounts(last_display_fds,1,1);
					}
					else
					{
						torque = ConvertFromCounts(hold_fds,1,1);
					}

					if ( fabs(torque - display_torque) > 0.75*(float)TorqueRounding )
					{
						if (RefreshCounter == RefreshPeriod )
						{
							display_torque = round(torque / (float)TorqueRounding)*(float)TorqueRounding;
						}
						LCD_displayFloat1D(display_torque);
					}
					if ( (int)display_torque > Max_Torque )
					{
						LCD_dashes();
					}
					else if (( holdState !=  HOLD_OFF ) && ( (int)current_pressure < 5) )
					{
						holdState = HOLD_OFF;
						LCD_dashes();
					}
					else if (( holdState !=  HOLD_OFF ) && ( (int)hold_pressure < 20) )
					{
						LCD_dashes();
					}
					else if (( holdState ==  HOLD_OFF ) && ( (int)current_pressure < 20 ))
					{
						LCD_dashes();
					}
					else
					{
						LCD_displayFloat1D(display_torque);
					}

					break;
				}
				break;
			}
		}

		if ( power_state == P_FIELD_TORQUE_INCREMENT )
		{
			FieldTorqueIncrementStateMachine();
		}

		if ( power_state == P_FIELD_DISPLAY_REFRESH )
		{
			FieldDisplayRefreshStateMachine();
		}

		if ( power_state == P_FIELD_CALIBRATE)
		{
			FieldCalibrationStateMachine();
		}

		if ( power_state == P_FACTORY_CALIBRATE )
		{

			calibrationTimeout ++;
			auto_off_timer_ticks = 0;	// no auto timeout when calibrating

			if ( calibrationTimeout > CAL_TIMEOUT )
			{
				power_state = P_OFF;
			}
			LCD_calibration();
		}

		if ( power_state == P_MAX_SETUP )
		{
			MaxSetupStateMachine();
		}


		if ((RefreshCounter == RefreshPeriod ) || ( power_state > P_ON ))
		{
			LCD_paint();
		}
		if ( holdState == HOLD_BLINK )
		{
			LCD_clear(0);
			LCD_paint();
		}

		CmdParse();

#define noDEBUG_PRESSURE_DROP
#if defined(DEBUG_PRESSURE_DROP)
		sprintf(rxBuff,"%4.1f,%4.1f,%d,%d",current_pressure,steady_pressure,actuationState,actuationCount);
		rxIndex = 0;

		char ch = rxBuff[rxIndex++];
		while ( ch != 0 )
		{
			UART_Transmit(ch);
			ch = rxBuff[rxIndex++];
		};
		rxIndex = 0;
		rxBuff[0] = 0;


		UART_Transmit(0x0D);
		UART_Transmit(0x0A);

#endif



#if defined ( TEST_EEPROM)
		dummy1 = EEPROM_readLong(0);
		EEPROM_writeLong( 0, 0x55555555 );

		dummy1 = EEPROM_readLong(1);
		EEPROM_writeLong( 1, 0xAAAAAAAA );

		dummy1 = EEPROM_readLong(2);
		EEPROM_writeLong( 2, 0x12345678 );

		fdummy1=  EEPROM_readFloat(3);
		EEPROM_writeFloat( 3, (float)1234.5678 );

#endif



	}
}

/*
 * end of file
 */
