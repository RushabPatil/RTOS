#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "sysctl_pll.h"
#include "buttons.h"
#include <math.h>
#include "sampling.h"
#include "inc/tm4c1294ncpdt.h"  // for accessing the registers directly
#include "Crystalfontz128x128_ST7735.h"

volatile int32_t gADCBufferIndex = ADC_BUFFER_SIZE -1; // latest sample index
volatile uint16_t gADCBuffer[ADC_BUFFER_SIZE];          // circular buffer
volatile uint32_t gADCErrors= 0;
volatile uint16_t localBuffer[ADC_BUFFER_SIZE];          // circular buffer


void ADCInit(){

    //initialize ADC peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // initialize ADC0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1); // initialize ADC1 peripheral
    uint32_t pll_frequency = SysCtlFrequencyGet(CRYSTAL_FREQUENCY);
    uint32_t pll_divisor = (pll_frequency - 1) / (16 * ADC_SAMPLING_RATE) + 1; // round divisor up
    gADCSamplingRate = pll_frequency / (16 * pll_divisor); // actual sampling rate may differ from ADC_SAMPLING_RATE
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor); // only ADC0 has PLL clock divisor control
    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);
    // initialize ADC sampling sequence
    ADCSequenceDisable(ADC0_BASE, 0);
    ADCSequenceDisable(ADC1_BASE, 0);

    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0);

    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH13);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH17 | ADC_CTL_IE | ADC_CTL_END);  // Joystick VER(Y)
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END );

    ADCSequenceEnable(ADC0_BASE, 0);
    ADCSequenceEnable(ADC1_BASE, 0);

    ADCIntEnable(ADC1_BASE, 0);
    IntPrioritySet(INT_ADC1SS0, (ADC_INT_PRIORITY << (8 - NUM_PRIORITY_BITS)));
    IntEnable(INT_ADC1SS0);
}

/**
 *
 */
void ADC_ISR(void)
{
    ADC1_ISC_R = ADC_ISC_IN0; // clear ADC1sequence0interrupt flag in the ADCISCregister // not sure about this one
    if (ADC1_OSTAT_R & ADC_OSTAT_OV0) { // check for ADC FIFO overflow
        gADCErrors++;// count errors
        ADC1_OSTAT_R = ADC_OSTAT_OV0;   // clear overflow condition
        }
    gADCBuffer[
               gADCBufferIndex = ADC_BUFFER_WRAP(gADCBufferIndex + 1)
               ] = ADC1_SSFIFO0_R;               // read sample from the ADC1sequence0 FIFO}
}


/**finds the index of the rising edge
 * Returns the index value
 */
int RisingTrigger(void)        //search for rising edge trigger
{
    int x = gADCBufferIndex - Lcd_ScreenWidth/2;//half screen width

    int x_stop = x - ADC_BUFFER_SIZE/2;

    for (; x > x_stop; x--)
    {
        if(gADCBuffer[ADC_BUFFER_WRAP(x)] >= ADC_OFFSET && gADCBuffer[ADC_BUFFER_WRAP(x-1)] < ADC_OFFSET)
            break;
    }
    if (x == x_stop) // for loop ran to the end
        x = gADCBufferIndex - Lcd_ScreenWidth/2; // reset x back to how it was initialized
    return x;

}

/**finds the index of the falling edge
 * Returns the index value
 */
int FallingTrigger(void)        //search for rising edge trigger
{
    int x = gADCBufferIndex - Lcd_ScreenWidth/2;//half screen width

    int x_stop = x - ADC_BUFFER_SIZE/2;

    for (; x > x_stop; x--)
    {
        if(gADCBuffer[ADC_BUFFER_WRAP(x)] >= ADC_OFFSET && gADCBuffer[ADC_BUFFER_WRAP(x-1)] < ADC_OFFSET)
            break;
    }
    if (x == x_stop) // for loop ran to the end
        x = gADCBufferIndex - Lcd_ScreenWidth/2; // reset x back to how it was initialized
    return x;

}

/** Copies waveform in the local buffer
 */
void CopyWaveform(int triggerIndex)
{
    int i = 0;
    for (; i < Lcd_ScreenWidth; i++)
    {
        localBuffer[i] = gADCBuffer[ADC_BUFFER_WRAP(triggerIndex - Lcd_ScreenWidth + i)];
    }

}

uint32_t CpuLoadCalc(void)
{
    uint32_t i = 0;
    TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER3_BASE, TIMER_A);
    while(!(TimerIntStatus(TIMER3_BASE, false) & TIMER_TIMA_TIMEOUT))
        i++;
    return i;
}




