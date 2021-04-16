/**
 * main.c
 *
 * ECE 3849 Lab 0 Starter Project
 * Gene Bogdanov    10/18/2017
 *
 * This version is using the new hardware for B2017: the EK-TM4C1294XL LaunchPad with BOOSTXL-EDUMKII BoosterPack.
 *
 */

/* Include files description
 *
 *  <stdint.h> - This header defines a set of integral type aliases with specific width requirements,
 *along with macros specifying their limits and macro functions to create values of these types. - https://www.cplusplus.com/reference/cstdint/
 *
 *<stdbool.h> - This header is to add a bool type and the true and false values as macro definitions.
 *
 *"driverlib/fpu.h" - fpu.h - Prototypes for the floatint point manipulation routines.
 *
 *"driverlib/sysctl.h" - Prototypes for the system control driver.
 *
 *"driverlib/interrupt.h" -  Prototypes for the NVIC Interrupt Controller Driver.
 *
 *"Crystalfontz128x128_ST7735.h" - Display driver for the Crystalfontz128x128 display with ST7735 controller.
 *
 *<stdio.h> - this header defines three variable types, several macros,
 *            and various functions for performing input and output.
 */
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/fpu.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include "buttons.h"
#include <math.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "sampling.h"
#include "Crystalfontz128x128_ST7735.h"


#define PWM_FREQUENCY 20000 // PWM frequency = 20 kHz

uint32_t gSystemClock; // [Hz] system clock frequency - global variable
volatile uint32_t gTime = 8345; // time in hundredths of a second
extern volatile uint16_t localBuffer[];
uint32_t count_loaded;
uint32_t count_unloaded;
float cpu_load;
uint32_t rising = 0;

float fScale = (VIN_RANGE * PIXELS_PER_DIV)/((1 << ADC_BITS) * fVoltsPerDiv);

void signal_init();

int main(void){

    IntMasterDisable();

    // Enable the Floating Point Unit, and permit ISRs to use it
    // FPUs are designed specifically for carrying out operations on floating point numbers
    FPUEnable();
    FPULazyStackingEnable(); //Lazy Stacking - Enables floating point operations in the interrupt

    // Initialize the system clock to 120 MHz - How does this work?
    gSystemClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, 120000000);

    signal_init();

    Crystalfontz128x128_Init(); // Initialize the LCD display driver
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);// set screen orientation

    ADCInit();
    ButtonInit();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    TimerDisable(TIMER3_BASE, TIMER_BOTH);
    TimerConfigure(TIMER3_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER3_BASE, TIMER_A, (float)gSystemClock/50 - 0.5);

    count_unloaded = CpuLoadCalc();


    IntMasterEnable();


    tContext sContext; // declare tContext variable for initializing grlib - graphics library
    GrContextInit(&sContext, &g_sCrystalfontz128x128); // Initialize the grlib graphics context
    GrContextFontSet(&sContext, &g_sFontFixed6x8); // select font



    uint32_t time;  // local copy of gTime
    char str[50];   // string buffer
    // full-screen rectangle
    tRectangle rectFullScreen = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1};

    //long gButtonsBinary = decimalToBinary(gButtons);

    while (true) {

        //drawing grid

        GrContextForegroundSet(&sContext, ClrBlack);
        GrRectFill(&sContext, &rectFullScreen); // fill screen with black
        time = gTime; // read shared global only once
        snprintf(str, sizeof(str), "Time = %02u:%02u:%02u\n", time/6000, (time%6000)/100, time%100); // convert time to string
        GrContextForegroundSet(&sContext, ClrYellow); // yellow text
        //GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 50, /*opaque*/ false);

        int x = 0;
           x = RisingTrigger();
           CopyWaveform(x);

           int i = 0;
           //draw the waveform
           for(;i < Lcd_ScreenWidth -1; i++){
               int y = LCD_VERTICAL_MAX/2 -(int)roundf(fScale * ((int)localBuffer[i] -ADC_OFFSET));
               int y2 = LCD_VERTICAL_MAX/2 -(int)roundf(fScale * ((int)localBuffer[i+1] -ADC_OFFSET));
               GrLineDraw(&sContext, i, y, i+1, y2);
           }

        count_loaded = CpuLoadCalc();

        cpu_load = 1.0f - (float)count_loaded/count_unloaded;

        snprintf(str, sizeof(str), "CPU Load = %0.1f%%", cpu_load*100);

        GrContextForegroundSet(&sContext, ClrYellow); // yellow text
        GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 10, /*opaque*/ false);

        char c;
        while(fifo_get(&c)){
            rising = ~rising;
            GrStringDraw(&sContext, &c, 1, 0, 30, false);
        }

        //
        if(rising == 0){
            snprintf(str, sizeof(str), "20 us 1V Rising");
        }
        else{
            snprintf(str, sizeof(str), "20 us 1V Falling");
        }


        //
        GrContextForegroundSet(&sContext, ClrYellow); // yellow text
        GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 110, /*opaque*/ false);

        GrContextForegroundSet(&sContext, ClrBlue); // yellow text
        int x_temp = 0;
        int y_temp = 0;
        for(; x_temp < 120; x_temp = x_temp + 20){

          GrLineDraw(&sContext,x_temp, 0, x_temp, 128);
           }
        for(; y_temp < 120; y_temp = y_temp + 20){

          GrLineDraw(&sContext,0, y_temp, 128, y_temp);

           }



       // GrStringDraw(&sContext, str, /*length*/ -1, /*x*/ 0, /*y*/ 10, /*opaque*/ false);



        GrFlush(&sContext); // flush the frame buffer to the LCD
    }
}

void signal_init(){
    // configure M0PWM2, at GPIO PF2, BoosterPack 1 header C1pin 2
    // configure M0PWM3, at GPIO PF3, BoosterPack 1 header C1 pin 3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinConfigure(GPIO_PF2_M0PWM2);
    GPIOPinConfigure(GPIO_PF3_M0PWM3);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

    // configure the PWM0 peripheral, gen 1,outputs 2 and 3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_1);// use system clock without division
    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, roundf((float)gSystemClock/PWM_FREQUENCY));
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, roundf((float)gSystemClock/PWM_FREQUENCY*0.4f));
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3, roundf((float)gSystemClock/PWM_FREQUENCY*0.4f));
    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT | PWM_OUT_3_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
}


