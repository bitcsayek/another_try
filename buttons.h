/*
 * buttons.h
 *
 *  Created on: Sep 12, 2013
 *      Author: mark
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

int BUTTON_read_IO(void);

int BUTTON_timed_read_IO_init(void);
int BUTTON_timed_read_IO(void);

int BUTTON_read_PT_UP(void); // also UP
int BUTTON_read_EM_DOWN(void); // also DOWN

int BUTTON_wait_for_on(void);

extern int current_IO;
extern int current_PT;
extern int current_EM;

#define BUTTON_SHORT_LONG_THRESHOLD 32
#define BUTTON_SHORT_PRESS 1
#define BUTTON_LONG_PRESS 2


#endif /* BUTTONS_H_ */
