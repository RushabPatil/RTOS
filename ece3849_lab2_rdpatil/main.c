/*
 * ECE 3849 Lab2 starter project
 *
 * Gene Bogdanov    9/13/2017
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/cfg/global.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"

uint32_t gSystemClock = 120000000; // [Hz] system clock frequency

/*
 *  ======== main ========
 */
int main(void)
{
    IntMasterDisable();

    // hardware initialization goes here

    /* Start BIOS */
    BIOS_start();

    return (0);
}

void task0_func(UArg arg1, UArg arg2)
{
    IntMasterEnable();

    while (true) {
        // do nothing
    }
}


//highest priority
/**
 * @brief 
 * 
 * @param arg1 
 * @param arg2 
 */
void waveformTask_func(UArg arg1, UArg arg2)
{
}


//high priority
/**
 * @brief 
 * 
 * @param arg1 
 * @param arg2 
 */
void buttonTask_func(UArg arg1, UArg arg2)
{

}

//mid priority
/**
 * @brief 
 * 
 * @param arg1 
 * @param arg2 
 */
void userInputTask_func(UArg arg1, UArg arg2)
{
}


//low priority
/**
 * @brief 
 * 
 * @param arg1 
 * @param arg2 
 */
void displayTask_func(UArg arg1, UArg arg2)
{
}

//lowest priority
/**
 * @brief 
 * 
 * @param arg1 
 * @param arg2 
 */
void processingTask_func(UArg arg1, UArg arg2)
{
}

