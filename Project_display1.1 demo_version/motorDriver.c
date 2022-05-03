/*
 * motorDriver.c
 *
 *  Created on: 02-May-2022
 *      Author: HP
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_adc.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "motorDriver.h"
#include "touchscreenInterrupt.h"

int motor_speed_delay = 50,motor_timer=1, motor_direction = 0;


void driveMotor()
{
    GPIO_PORTB_DIR_R = 0x0F;

    GPIO_PORTB_DEN_R = 0x0F;
    int i = 0;
    if (motor_direction)
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02);


        for (i = 1; i <= 600; i++)

        {
            if (!reset)
            {
                //to rotate in anti-clockwise
                GPIO_PORTB_DATA_R = 0x08;
                delay(motor_speed_delay); //delay inversely proportional speed. 1ms is max speed.
                GPIO_PORTB_DATA_R = 0x04;
                delay(motor_speed_delay);
                GPIO_PORTB_DATA_R = 0x02;
                delay(motor_speed_delay);
                GPIO_PORTB_DATA_R = 0x01;
                delay(motor_speed_delay);
            }

        }

    }
    else if (!motor_direction)
    {
        for (i = 1; i <= 600; i++)
        {
            if (!reset)
            {
                //to rotate in clockwise
                GPIO_PORTB_DATA_R = 0x01;
                delay(motor_speed_delay); //delay inversely proportional speed. 1ms is max speed.
                GPIO_PORTB_DATA_R = 0x02;
                delay(motor_speed_delay);
                GPIO_PORTB_DATA_R = 0x04;
                delay(motor_speed_delay);
                GPIO_PORTB_DATA_R = 0x08;
                delay(motor_speed_delay);
            }

        }

    }
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00);
    GPIO_PORTB_DATA_R = 0x00;
}

void delay(int n)
{
    int i, j;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < 3180; j++)
        {
            ;
        }
    }
}
