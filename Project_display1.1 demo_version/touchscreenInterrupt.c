/*
 * touchscreenInterrupt.c
 *
 *  Created on: 02-May-2022
 *      Author: Anupam
 */
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/flash.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/rom.h"
#include "driverlib/gpio.h"
#include "touchscreenInterrupt.h"


#define PINS  GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4

void DisableInterrupts(void);
void EnableInterrupts(void);
void WaitForInterrupt(void);



int blink,reset;

void init_PORTF_interrupt(void)
{
    GPIO_PORTF_LOCK_R |= 0x4C4F434B; /* 2) unlock GPIO Port F */
    GPIO_PORTF_CR_R |= 0x01; /* allow changes to PF4-0 */
    GPIO_PORTF_AMSEL_R |= 0x00; /* 3) disable analog functionality on PF */
    GPIO_PORTF_PCTL_R |= 0x00000000; /* 4) Clear the bits to select regular digital function PCTL GPIO on PF4-0 */
    GPIO_PORTF_DIR_R |= 0x0; /* 5) PF4,PF0 in, PF3-1 out */
    GPIO_PORTF_AFSEL_R |= 0x00; /* 6) disable alternate function on PF7-0 */
    GPIO_PORTF_PUR_R |= 0x01; /* enable pull-up on PF0 and PF4 (active low)*/
    GPIO_PORTF_DEN_R |= 0x01; /* 7) enable digital I/O on PF4-0 */

    //Enable the interrupt for a specific peripheral module
    GPIO_PORTF_IS_R &= ~0x01; /*  PF4 is edge-sensitive -0:edge ;1:level*/
    GPIO_PORTF_IBE_R &= ~0x01; /*  PF4 is not both edges */
    GPIO_PORTF_IEV_R &= ~0x01; /*  PF4 falling edge event as sw1 and sw2 are active low input */
    GPIO_PORTF_ICR_R = 0x01; /*  Clear flag for PF4 */
    GPIO_PORTF_IM_R |= 0x01; /*  unmask(arm) interrupt on PF4 */

    //Enable interrupt at the NVIC module
    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF1FFFFF) | 0x00A00000; /*  priority 5 */
    NVIC_EN0_R = 0x40000000; /*IRQ number of the Port F is 30, the bit 30 in the NVIC_EN0_R register should be set to 1 to enable this port*/

    //Enable the interrupt globally
    EnableInterrupts(); /* Enable global Interrupt flag (I) */
}
void GPIOPortF_Handler(void)
{

    volatile int readback;


     if ((GPIO_PORTF_RIS_R & 0x01) == 0x01)
    {

        GPIO_PORTF_ICR_R |= 0x01;     /* clear PF0 int */
        readback = GPIO_PORTF_ICR_R; /* a read to force clearing of interrupt flag */
        readback = readback;
        reset=1;
        blink=0;

    }
}

//void init_PORTE(void)
//{
//
//    // PORT C should not be interferred as it is required to communicate with ICDI connection interface
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);// 1) activate clock for Port E
//
//    GPIO_PORTE_AMSEL_R = 0x00;      // 3) disable analog functionality on PF
//    GPIO_PORTE_PCTL_R = 0x0; // 4) Clear the bits to select regular digital function PCTL GPIO on PF4-0
//    GPIO_PORTE_DIR_R = 0x00;        // 5) PF4,PF0 in, PF3-1 out
//    GPIO_PORTE_AFSEL_R = 0x00;      // 6) disable alternate function on PF7-0
//    GPIO_PORTE_DEN_R = 0x1F;        // 7) enable digital I/O on PF4-0
//
//    // Enable the interrupt for a specific peripheral module
//    GPIO_PORTE_IS_R &= ~0x1F;  // PF4 & PF0 is edge-sensitive -0:edge ;1:level
//    GPIO_PORTE_IBE_R &= ~0x1F; // PF4 & PF0 is not both edges
//    GPIO_PORTE_IEV_R &= 0x1F; // PE0-4 rising edge event
//    GPIO_PORTE_ICR_R = 0x1F;   // Clear flag for PE
//    GPIO_PORTE_IM_R |= 0x1F;   // unmask(arm) interrupt on PF4 & PF0
//
//    // Enable interrupt at the NVIC module
//    NVIC_PRI1_R = (NVIC_PRI1_R & 0xFFFFFF1F) | 0x000000A0; // priority 5
//    NVIC_EN0_R = 0x00000010; // IRQ number of the Port E is 4, the bit 4 in the NVIC_EN0_R register should be set to 1 to enable this port
//
//    // Enable the interrupt globally
//    EnableInterrupts();
//}
//
//void GPIOPortE_Handler(void)
//{
//    int pause_count = 0;
//    volatile int readback;
//
//    if ((GPIO_PORTE_RIS_R & 0x01) == 0x01)
//    { // check if interrupt occurred on PF4
//
//        GPIO_PORTE_ICR_R = 0x01; /* clear PF4 int */
//        readback = GPIO_PORTE_ICR_R; /* a read to force clearing of interrupt flag */
//        readback = readback;
//        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2 | GPIO_PIN_3, 0x0E);
//
//    }
//
//    else if ((GPIO_PORTE_RIS_R & 0x02) == 0x02)
//    {
//
//        GPIO_PORTE_ICR_R = 0x02; /* clear PF0 int */
//        readback = GPIO_PORTE_ICR_R; /* a read to force clearing of interrupt flag */
//        readback = readback;
//        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_3, 0x00);
//    }
//}
/*********** DisableInterrupts ***************
 *
 * disable interrupts
 *
 * inputs:  none
 * outputs: none
 */

void DisableInterrupts(void)
{
    __asm("    CPSID  I\n");
}

/*********** EnableInterrupts ***************
 *
 * emable interrupts
 *
 * inputs:  none
 * outputs: none
 */

void EnableInterrupts(void)
{
    __asm("    CPSIE  I\n");
}
