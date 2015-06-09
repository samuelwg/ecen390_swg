/*
 * main.c
 *
 *  Created on: Dec 19, 2014
 *      Author: hutch
 */

#include <stdio.h>
//#include "platform.h"
#include "supportFiles/leds.h"
#include "supportFiles/globalTimer.h"
#include "supportFiles/interrupts.h"
#include "supportFiles/intervalTimer.h"
#include "supportFiles/buttons.h"
#include "supportFiles/switches.h"
#include <stdbool.h>
#include <stdlib.h>
#include "isr.h"

#define TOTAL_SECONDS 3

int main() {
	// We want to use the interval timers.
	intervalTimer_initAll();
	intervalTimer_resetAll();
	intervalTimer_testAll();
	printf("Laser Tag Test Program\n\r");
	// Find out how many timer ticks per second.
	u32 privateTimerTicksPerSecond = interrupts_getPrivateTimerTicksPerSecond();
	printf("private timer ticks per second: %ld\n\r", privateTimerTicksPerSecond);
	// Initialize the GPIO LED driver and print out an error message if it fails (argument = true).
	// The LD4 LED provides a heart-beat that visually verifies that interrupts are running.
	leds_init(true);
	// Init all interrupts (but does not enable the interrupts at the devices).
	// Prints an error message if an internal failure occurs because the argument = true.
	interrupts_initAll(true);
	// Enables the main interrupt on the time.
	interrupts_enableTimerGlobalInts();
	// Start the private ARM timer running.
	interrupts_startArmPrivateTimer();
	printf("This program will run for %d seconds and print out statistics at the end of the run.\n\r",
			TOTAL_SECONDS);
	printf("Starting timer interrupts.\n\r");
	// Enable global interrupt of System Monitor. The system monitor contains the ADC. Mainly to detect EOC interrupts.
	interrupts_enableSysMonGlobalInts();
	// Start a duration timer and compare its timing against the time computed by the timerIsr().
	// Assume that ENABLE_INTERVAL_TIMER_0_IN_TIMER_ISR is defined in interrupts.h so that time spent in timer-isr is measured.
	intervalTimer_start(1);
	int countInterruptsViaInterruptsIsrFlag = 0;
	// Enable interrupts at the ARM.
	interrupts_enableArmInts();
	// Wait until TOTAL seconds have expired. globalTimerTickCount is incremented by timer isr.
	while (interrupts_isrInvocationCount() < (TOTAL_SECONDS * privateTimerTicksPerSecond)) {
		if (interrupts_isrFlagGlobal) {				// If this is true, an interrupt has occured (at least one).
			countInterruptsViaInterruptsIsrFlag++;	// Note that you saw it.
			interrupts_isrFlagGlobal = 0;			// Reset the shared flag.
			// Place non-timing critical code here.
		}
	}
    interrupts_disableArmInts();	// Disable ARM interrupts.
 	intervalTimer_stop(1);			// Stop the interval timer.

	double runningSeconds, isrRunningSeconds;
	intervalTimer_getTotalDurationInSeconds(1, &runningSeconds);
	printf("Total run time as measured by interval timer in seconds: %g.\n\r", runningSeconds);
	intervalTimer_getTotalDurationInSeconds(0, &isrRunningSeconds);
	printf("Measured run time in timerIsr (using interval timer): %g.\n\r", isrRunningSeconds);
	printf("Detected interrupts via global flag: %d\n\r", countInterruptsViaInterruptsIsrFlag);
    printf("During %d seconds, an average of %7.3f ADC samples were collected per second.\n\r",
    		TOTAL_SECONDS, (float) isr_getTotalAdcSampleCount() / (float) TOTAL_SECONDS);
   return 0;
}







