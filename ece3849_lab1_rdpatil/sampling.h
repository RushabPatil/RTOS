
#ifndef SAMPLING_H_
#define SAMPLING_H_

#include "sysctl_pll.h"



#define ADC_SAMPLING_RATE 1000000   // [samples/sec] desired ADC sampling rate
#define CRYSTAL_FREQUENCY 25000000  // [Hz] crystal oscillator frequency used to calculate clock rates

#define ADC_INT_PRIORITY 0
#define VIN_RANGE 3.3
#define PIXELS_PER_DIV 20
#define ADC_BITS 12
#define ADC_OFFSET 2048
#define fVoltsPerDiv 1

#define ADC_BUFFER_SIZE 2048
#define ADC_BUFFER_WRAP(i) ((i) & (ADC_BUFFER_SIZE -1))// index wrapping macro




void ADCInit(void);
void ADC_ISR(void);
int RisingTrigger(void);
void CopyWaveform(int triggerIndex);
uint32_t CpuLoadCalc(void);


#endif
