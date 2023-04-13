/*
 * parser.h
 *
 *  Created on: Oct 23, 2013
 *      Author: mark
 */

#ifndef PARSER_H_
#define PARSER_H_

#define RX_BUFF_SIZE 64
extern unsigned int rxIndex;
extern char rxBuff[RX_BUFF_SIZE];


void CmdParse(void);


#endif /* PARSER_H_ */
