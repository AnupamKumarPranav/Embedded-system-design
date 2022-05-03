/*
 * touchscreenInterrupt.h
 *
 *  Created on: 02-May-2022
 *      Author: Anupam
 */

#ifndef TOUCHSCREENINTERRUPT_H_
#define TOUCHSCREENINTERRUPT_H_


extern int blink,reset;
//void init_PORTE(void);
//void GPIOPortE_Handler(void);
void GPIOPortF_Handler();
void init_PORTF_interrupt(void);



#endif /* TOUCHSCREENINTERRUPT_H_ */
