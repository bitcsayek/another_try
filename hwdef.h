/*
 * hwdef.h
 *
 *  Created on: Sep 11, 2013
 *      Author: mark
 *
 *  Modified to read LTC3335 Buck/Boost regulator w/I2C Coulomb counter
 *      Date: 30 June, 2022
 *      Author: mw
 */



#ifndef HWDEF_H_
#define HWDEF_H_

//#define __LTC3335   //added mw 30 June, 2022
/*************************************************/
/*************************************************/


#include <msp430f6726.h>
#include "version.h"

#define TRUE ( 1==1 )
#define FALSE (!TRUE);

#ifndef __LTC3335

#define BAT_TS_EN_PORT P4OUT
#define BAT_TS_EN_BIT BIT1

#define BAT_TS_PORT P1IN
#define BAT_TS_BIT BIT2

#define ADPTR_ON_PORT P1IN
#define ADPTR_ON_BIT BIT6

#define CHARGING_B_PORT P2IN
#define CHARGING_B_BIT BIT7

#endif

#define IO_SWITCH_EN_PORT P4OUT
#define IO_SW_IO_EN_BIT BIT3
#define IO_SWS_EN_BIT (BIT3|BIT2)

#define IO_SW_PORT P2IN
#define IO_IO_SW_BIT BIT5
#define IO_PT_SW_BIT BIT6
#define IO_EM_SW_BIT BIT4

#define SEN_EN_PORT P4OUT
#define SEN_EN_BIT BIT0

#define EPROM_PORT P3OUT
#define EPROM_CS_B_BIT BIT7
#define EPROM_WP_B_BIT BIT6


// threshold voltage count for the battery icon
#ifndef __LTC3335
#define VT_1        470	    // 3.9	above this 3 bars
#define VT_2        470	    // 3.8	above this 2 bars
#define VT_3        470	    // 3.7	above this 1 bar
#define VT_4        470	    // 3.6  above this 0 bars, below this - flash and disable
#define VT_4_hyst   470	    // 3.65 voltage required to go from dead battery to zero bars
#else

//approximate alkaline battery capacities in mA*hr by vendor
#define ZEUS        1350    //available through Digikey
#define ENERGIZER   1250    //available most stores
#define DURACELL    1150    //available most stores

#define ZEUS_CAP    (ZEUS/1000)*3600
#define ENER_CAP    (ENERGIZER/1000)*3600
#define DURA_CAP    (DURACELL/1000)*3600

//number of Coulombs burned - scaled to fit into 8 bits -> ((target/lsb weight)/3  ex. (1125/5.492)/3 = 68
//see spreadsheet LTC3335 HW Config. for the below
//based upon a Duracell battery - 1250mA*hr => 4500C
#define CL_CNT_4    253 //4185   //0 bars - flashing and disabled above this count ~7% capacity remains any percent higher overflows 8 bits
#define CL_CNT_3    204 //3375   //1 bar
#define CL_CNT_2    136 //2250   //2 bars
#define CL_CNT_1    68  //1125   //fully charged at or below this count: 3 bars


//Alarm types for LTC3335
//the first two alarms should never occur - see pages 16 & 17 in LTC3335 datasheet for more details
#define INDUCTOR_FAULT  0x01    //bit 0 of register D is high indicating MOSFET pair AC(on) time overflow
#define COUNTER_FAULT   0x02    //bit 1 of register D is high indicating coulomb counter overflow
#define PRESET_FAULT    0x04    //bit 2 of register D is high indicating ripple counter value is equal to or greater than threshold set in register B

//Coulomb count error for 100mA Ipeak
//Adjusted coulomb count = raw count *(1/(1+error)) + 5.96mA*hr * elapsed time/qlsb_m  -  see LTC3335 HW Config.xlxs spreadsheet for qlsb_m value
//5.96mA value is mA*hr error due to sleep current per year from datasheet page 22
//Without an RTC we must rely on the raw coulomb count
#define COULOMB_ERROR   0.0125  //from plot G39 LTC3335 datasheet page 11
#define QLSB_WEIGHT     5.492   //qlsb_m value calculated from Qlsb/Perscale - see spreadsheet LTC3335 HW Config.xlxs for calculations
#define PRESCALE        9       //see spreadsheet for calculation

#endif


#endif /* HWDEF_H_ */
