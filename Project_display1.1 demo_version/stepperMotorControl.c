#include <stdint.h>
#include <stdbool.h>
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
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/checkbox.h"
#include "grlib/container.h"
#include "grlib/pushbutton.h"
#include "grlib/radiobutton.h"
#include "grlib/slider.h"
#include "utils/ustdlib.h"
#include "Kentec320x240x16_ssd2119_spi.h"
#include "touch.h"
#include "images.h"
#include "touchscreenInterrupt.h"
#include "motorDriver.h"

//! The use of LCD BoosterPack (BOOSTXL-K350QVG-S1) requires resistors R9 and
//! R10 to be removed from the EK-TM4C123GXL Launchpad.
//*****************************************************************************

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// The DMA control structure table.
//
//*****************************************************************************
#ifdef ewarm
#pragma data_alignment = 1024
tDMAControlTable psDMAControlTable[64];
#elif defined(ccs)
#pragma DATA_ALIGN(psDMAControlTable, 1024)
tDMAControlTable psDMAControlTable[64];
#else
tDMAControlTable psDMAControlTable[64] __attribute__((aligned(1024)));
#endif

//*****************************************************************************
//
// Forward declarations for the globals required to define the widgets at
// compile-time.
//
//*****************************************************************************
void OnButtonMinus(tWidget *psWidget);
void OnButtonPlus(tWidget *psWidget);
void OnStart(tWidget *psWidget);
void OnStop(tWidget *psWidget);
void OnClockwise(tWidget *psWidget);
void OnAntiClockwise(tWidget *psWidget);
void delayMs(int n);
void messageContextReset();
int timer_count, speed_plus_count, speed_minus_count, stop;

tContext sContext;
tRectangle sMsgRect;
extern tPushButtonWidget g_increase, g_decrease, g_clockwise, g_antiClokwise;

// Containers for control widgets grouping

Container(
        g_sContainer2,
        WIDGET_ROOT,
        0,
        &g_increase,
        &g_sKentec320x240x16_SSD2119,
        5,
        135,
        170,
        90,
        (CTR_STYLE_OUTLINE | CTR_STYLE_FILL | CTR_STYLE_TEXT | CTR_STYLE_TEXT_CENTER),
        ClrWheat, ClrBlack, ClrBlack, &g_sFontCm18, "Speed control");
Container(
        g_sContainer1,
        WIDGET_ROOT,
        &g_sContainer2,
        &g_clockwise,
        &g_sKentec320x240x16_SSD2119,
        5,
        35,
        170,
        90,
        (CTR_STYLE_OUTLINE | CTR_STYLE_FILL | CTR_STYLE_TEXT | CTR_STYLE_TEXT_CENTER),
        ClrWheat, ClrBlack, ClrBlack, &g_sFontCm18, "Direction Control");

//*****************************************************************************
// The buttons and text across the bottom of the screen.
//*****************************************************************************
RectangularButton(g_increase, &g_sContainer2, &g_decrease, 0,
                  &g_sKentec320x240x16_SSD2119, 20, 155, 50, 50,
                  (PB_STYLE_IMG | PB_STYLE_TEXT), ClrLightYellow,
                  ClrLightYellow, 0, ClrSilver, &g_sFontCm20, "+",
                  g_pui8Blue50x50, g_pui8Blue50x50Press, 0, 0, OnButtonPlus);

RectangularButton(g_decrease, &g_sContainer2, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 110, 155, 50, 50,
                  (PB_STYLE_IMG | PB_STYLE_TEXT), ClrLightYellow,
                  ClrLightYellow, 0, ClrSilver, &g_sFontCm20, "-",
                  g_pui8Blue50x50, g_pui8Blue50x50Press, 0, 0, OnButtonMinus);

RectangularButton(g_clockwise, &g_sContainer1, &g_antiClokwise, 0,
                  &g_sKentec320x240x16_SSD2119, 20, 55, 50, 50,
                  (PB_STYLE_IMG | PB_STYLE_TEXT), ClrLightYellow,
                  ClrLightYellow, 0, ClrSilver, &g_sFontCm20, "Clk",
                  g_pui8Blue50x50, g_pui8Blue50x50Press, 0, 0, OnClockwise);

RectangularButton(g_antiClokwise, &g_sContainer1, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 110, 55, 50, 50,
                  (PB_STYLE_IMG | PB_STYLE_TEXT), ClrLightYellow,
                  ClrLightYellow, 0, ClrSilver, &g_sFontCm20, "AClk",
                  g_pui8Blue50x50, g_pui8Blue50x50Press, 0, 0, OnAntiClockwise);

RectangularButton(g_start, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 200, 30, 50,
                  50, (PB_STYLE_IMG | PB_STYLE_TEXT), ClrLightYellow,
                  ClrLightYellow, 0, ClrSilver, &g_sFontCm14, "START",
                  g_pui8Blue50x50, g_pui8Blue50x50Press, 0, 0, OnStart);

RectangularButton(g_stop, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 200, 100, 50,
                  50, (PB_STYLE_IMG | PB_STYLE_TEXT), ClrBlack, ClrBlack, 0,
                  ClrSilver, &g_sFontCm12, "STOP", g_pui8Blue50x50,
                  g_pui8Blue50x50Press, 0, 0, OnStop);

void messageContextReset()
{
    GrContextForegroundSet(&sContext, ClrMediumVioletRed);
    GrRectFill(&sContext, &sMsgRect);
    //
    // Put a white box around the banner.
    //
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sMsgRect);
}

void OnStart(tWidget *psWidget)
{
    reset = 0;
    messageContextReset();

    blink = 1;
    motor_timer = 1;
    stop=0;

}
void OnStop(tWidget *psWidget)
{
    reset = 0;
    messageContextReset();
    blink = 0;
    stop = 1;
    messageContextReset();
    GrContextFontSet(&sContext, &g_sFontCm16);
    GrStringDrawCentered(&sContext, "Stopped", -1, 260, 190, 0);
}
void OnClockwise(tWidget *psWidget)
{
    reset = 0;
    messageContextReset();
    GrContextFontSet(&sContext, &g_sFontCm16);
    GrStringDrawCentered(&sContext, "Clk", -1, 260, 190, 0);
    motor_direction = 0;
}

void OnAntiClockwise(tWidget *psWidget)
{
    reset = 0;
    messageContextReset();
    GrContextFontSet(&sContext, &g_sFontCm16);
    GrStringDrawCentered(&sContext, "AntiClk", -1, 260, 190, 0);
    motor_direction = 1;
}
void OnButtonPlus(tWidget *psWidget)
{
    reset = 0;
    messageContextReset();
    switch (speed_plus_count)
    {
    case 0:
        motor_speed_delay = 20;
        speed_plus_count = 1;
        GrContextFontSet(&sContext, &g_sFontCm16);
        GrStringDrawCentered(&sContext, "Speed+", -1, 260, 190, 0);
        break;

    case 1:
        motor_speed_delay = 10;
        speed_plus_count = 2;
        GrContextFontSet(&sContext, &g_sFontCm16);
        GrStringDrawCentered(&sContext, "Speed++", -1, 260, 190, 0);
        break;

    case 2:
        motor_speed_delay = 3;
        speed_plus_count = 0;
        GrContextFontSet(&sContext, &g_sFontCm16);
        GrStringDrawCentered(&sContext, "MAX", -1, 260, 190, 0);
        break;

    default:
        break;
    }
}
void OnButtonMinus(tWidget *psWidget)
{
    reset = 0;
    messageContextReset();

    switch (speed_minus_count)
    {
    case 0:
        motor_speed_delay = 70;
        speed_minus_count = 1;
        GrContextFontSet(&sContext, &g_sFontCm16);
        GrStringDrawCentered(&sContext, "Speed-", -1, 260, 190, 0);
        break;

    case 1:
        motor_speed_delay = 100;
        speed_minus_count = 2;
        GrContextFontSet(&sContext, &g_sFontCm16);
        GrStringDrawCentered(&sContext, "Speed--", -1, 260, 190, 0);
        break;

    case 2:
        motor_speed_delay = 150;
        speed_minus_count = 0;
        GrContextFontSet(&sContext, &g_sFontCm16);
        GrStringDrawCentered(&sContext, "MIN", -1, 260, 190, 0);

        break;

    default:
        break;
    }
}

int main(void)
{

    tRectangle sRect;
    uint32_t ui32SysClock;

    FPUEnable();
    FPULazyStackingEnable();

    //
    // Set the clock to 40Mhz derived from the PLL and the external oscillator
    //
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
    SYSCTL_OSC_MAIN);

    ui32SysClock = SysCtlClockGet();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
    GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00);
    //
    // Initialize the display driver.
    //
    Kentec320x240x16_SSD2119Init(ui32SysClock);
    //
    // Initialize the graphics context.
    //
    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);
    //
    // Fill the top 24 rows of the screen with blue to create the banner.
    //
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 23;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);
    //
    // Put a white box around the banner.
    //
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sRect);
    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&sContext, &g_sFontCm20);
    GrStringDrawCentered(&sContext, "Stepper motor control", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 8, 0);

    // Graphics context for Mesage display

    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);

    sMsgRect.i16XMin = 200;
    sMsgRect.i16YMin = 170;
    sMsgRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sMsgRect.i16YMax = 210;
    GrContextForegroundSet(&sContext, ClrMediumVioletRed);
    GrRectFill(&sContext, &sMsgRect);
    //
    // Put a white box around the banner.
    //
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sMsgRect);
    //
    // Put the status messages in the middle of the banner.
    //
    GrContextFontSet(&sContext, &g_sFontCm20);
    GrStringDrawCentered(&sContext, "Welcome", -1, 260, 190, 0);
    //
    // Configure and enable uDMA
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    SysCtlDelay(10);
    uDMAControlBaseSet(&psDMAControlTable[0]);
    uDMAEnable();
    //
    // Initialize the touch screen driver and have it route its messages to the widget tree.
    //
    TouchScreenInit(ui32SysClock);
    TouchScreenCallbackSet(WidgetPointerMessage);
    //
    // Add the container and button widgets to the widget tree.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget*) &g_sContainer1);
    WidgetAdd(WIDGET_ROOT, (tWidget*) &g_sContainer2);
    WidgetAdd(WIDGET_ROOT, (tWidget*) &g_start);
    WidgetAdd(WIDGET_ROOT, (tWidget*) &g_stop);

    //
    // Issue the initial paint request to the widgets.
    //
    WidgetPaint(WIDGET_ROOT);

    init_PORTF_interrupt();

    //
    //  Loop forever handling widget messages.
    //
    while (1)
    {

        // Process any messages in the widget message queue.

        WidgetMessageQueueProcess();
        if (!stop)
        {
            if (blink)
            {

                GrContextFontSet(&sContext, &g_sFontCm20);
                GrStringDrawCentered(&sContext, "Running", -1, 260, 190, 0);

                driveMotor();

                if (reset)
                {
                    motor_timer = 0;
                    break;
                }

            }
        }

    }
}
