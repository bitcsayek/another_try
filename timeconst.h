/*
 * timeconst.h
 *
 *  Created on: Jul 12, 2017
 *      Author: mark
 */

#ifndef TIMECONST_H_
#define TIMECONST_H_

#define QUARTER_SECOND ( 8 )
#define HALF_SECOND ( 16 )
#define TWO_SECONDS ( 2*32 )
#define THREE_SECONDS ( 3*32 )
#define TEN_SECONDS ( 10*32 )

#define HOLD_FREEZE_RELEASE_TICKS ( TWO_SECONDS )

#define ONE_MINUTE ( 60*32 )
#define TWO_MINUTES ( 2*ONE_MINUTE )
#define TEN_MINUTES ( 10*ONE_MINUTE)

#define CAL_TIMEOUT TEN_MINUTES

#define AUTO_OFF_MINUTES 3
#define AUTO_OFF_TICKS (32*60*AUTO_OFF_MINUTES)	// 3 minutes



#endif /* TIMECONST_H_ */
