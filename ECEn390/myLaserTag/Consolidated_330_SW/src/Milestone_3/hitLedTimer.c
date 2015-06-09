/*
 * hitLedTimer.c
 *
 *  Created on: Feb 19, 2015
 *      Author: samuelwg
 */


/*
 * hitLedTimer.c
 *
 *  Created on: Jan 23, 2015
 *      Author: hutch
 */



#include "hitLedTimer.h"
#include "supportFiles/switches.h"
#include <stdio.h>
#include "supportFiles/leds.h"
#include "supportFiles/globalTimer.h"
#include "supportFiles/interrupts.h"
#include "supportFiles/intervalTimer.h"
#include "supportFiles/mio.h"
#include "supportFiles/utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include "isr.h"
// The MIO system is the PS GPIO that communicates with the MIO pins on the ZYBO board.
// MIO pins: 13, 10, 11, 12, 0, 9, 14, 15 are connected to JF pins: 1, 2, 3, 4, 7, 8, 9, 10.


#define HIT_LED_TIMER_MIO_PIN 11
//Illuminate hit for 1/2 second. 1/.5 = 2.  100kHz/2 = 50000
#define HIT_LED_HIGH_VALUE 1
#define HIT_LED_LOW_VALUE 0
#define HIT_LED_CLOCK_UP 50000

static volatile uint32_t LED_counter = 0;
static volatile bool timerStartFlag = false;


enum hitLedTimer_st_t {
	init_st,                // Start here, stay in this state for just one tick.
	wait_st,        		// Wait here for the led to turn on
	led_On_st,    			// Turn Led on
	end_st
} hitLed_currentState = init_st;
// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.


void hitLed_debugStatePrint() {
	static hitLedTimer_st_t previousState;
	static bool firstPass = true;
	// Only print the message if:
	// 1. This the first pass and the value for previousState is unknown.
	// 2. previousState != currentState - this prevents reprinting the same state name over and over.
	if (previousState != hitLed_currentState || firstPass) {
		firstPass = false;                // previousState will be defined, firstPass is false.
		previousState = hitLed_currentState;     // keep track of the last state that you were in.
		//    printf("Led Counter:%d\n\r", LED_counter);
		switch(hitLed_currentState) {            // This prints messages based upon the state that you were in.
		case init_st:
			printf("init_st\n\r");
			break;
		case wait_st:
			printf("wait_st\n\r");
			break;
		case led_On_st:
			printf("Led_ON_st\n\r");
			break;
		case end_st:
			printf("end_st\n\r");
			break;

		}
	}
}

// Standard tick function.
void hitLedTimer_tick(){
	//	hitLed_debugStatePrint();
	switch(hitLed_currentState) {
	case init_st:

		break;
	case wait_st:
		break;
	case led_On_st:
		LED_counter++;
		break;
	case end_st:
		LED_counter = 0;
		break;
	default:
		printf("hitLedTimer tick state action: hit default\n\r");
		break;
	}

	// Transition states
	switch(hitLed_currentState) {
	case init_st:
		if(true == timerStartFlag){
			hitLed_currentState = wait_st;
		}
		break;
	case wait_st:
		hitLed_currentState = led_On_st;
		mio_writePin(HIT_LED_TIMER_MIO_PIN, HIT_LED_HIGH_VALUE);
		leds_write(1);
		break;
	case led_On_st:
		// Once it get's up to a half of a second then put the pin back to low and turn off led
		if(HIT_LED_CLOCK_UP <= LED_counter){
			hitLed_currentState = end_st;
			mio_writePin(HIT_LED_TIMER_MIO_PIN, HIT_LED_LOW_VALUE);
			leds_write(0);
		}
		break;
	case end_st:
		timerStartFlag = false;
		hitLed_currentState = init_st;
		break;
	default:
		printf("hitLedState state update: hit default\n\r");
		break;
	}


}



// Need to init things.
void hitLedTimer_init(){
	leds_init(true);
	mio_setPinAsOutput(HIT_LED_TIMER_MIO_PIN);
	leds_write(0);
}

// Calling this starts the timer.
void hitLedTimer_start(){
	timerStartFlag = true;


}

// Returns true if the timer is currently running.
bool hitLedTimer_running(){
	return timerStartFlag;
}

void hitLedTimer_runTest(){
	uint16_t TOTAL_SECONDS = 30;
	u32 privateTimerTicksPerSecond = interrupts_getPrivateTimerTicksPerSecond();
	while (interrupts_isrInvocationCount() < (TOTAL_SECONDS * privateTimerTicksPerSecond)) {
		if (interrupts_isrFlagGlobal) {
			hitLedTimer_start();
			while(hitLedTimer_running());
			utils_msDelay(500);
		}
	}

}

//#include <stdint.h>
//#include <stdbool.h>
//#include "hitLedTimer.h"
//#include "supportFiles/mio.h"
//#include "supportFiles/leds.h"
//#include <stdio.h>
//
//#define HIT_LED_TIMER_OUTPUT_PIN 11 // JF-3
//#define EXTERNAL_LED_OFF 0                                      // This value turns the LED off.
//#define EXTERNAL_LED_ON 1                                               // This value turns the LED on.
//#define LD0_ON 1                                                                                // On and off values for the LD0 led.
//#define LD0_OFF 0
//enum hitLedTimer_st_t {
//        init_st,                                                                                                // Standard init state.
//        waiting_for_timer_to_start_st,  // Waiting for someone to start the timer.
//        timer_running_st                                                                // Timer is running when in this state.
//};
//
//// Currentn state for the timer state machine.
//static hitLedTimer_st_t currentState = init_st;
//
//// When true, the timer starts running.
//static bool timerStartFlag = false;
//
//// Turns the LED on.
//void turnLedOn() {
//        mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, EXTERNAL_LED_ON);
//        leds_write(LD0_ON);
//}
//
//// Turns the LED off.
//void turnLedOff() {
//        mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, EXTERNAL_LED_OFF);
//        leds_write(LD0_OFF);
//}
//
//// Init the leds, set the hit-led pin as an output.
//void hitLedTimer_init() {
//        leds_init(true);
//        mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
//        turnLedOff();
//}
//
//// Call this to start the timer.
//void hitLedTimer_start() {
//        timerStartFlag = true;
//}
//
//// Checks to see if the timer is currently running.
//bool hitLedTimer_running() {
//        return currentState == timer_running_st;
//}
//
// void hitLedTimer_debugStatePrint() {
//        static hitLedTimer_st_t previousState;
//        static bool firstPass = true;
//        // Only print the message if:
//        // 1. This the first pass and the value for previousState is unknown.
//        // 2. previousState != currentState - this prevents reprinting the same state name over and over.
//        if (previousState != currentState || firstPass) {
//                firstPass = false;                // previousState will be defined, firstPass is false.
//                previousState = currentState;     // keep track of the last state that you were in.
//                switch(currentState) {            // This prints messages based upon the state that you were in.
//                case init_st:
//                        printf("hitLedTimer_init_st\n\r");
//                        break;
//                case waiting_for_timer_to_start_st:
//                        printf("hitLedTimer_waiting_for_timer_to_start_st\n\r");
//                        break;
//                case timer_running_st:
//                        printf("hitLedTimer_timer_running_st\n\r");
//                        break;
//                }
//        }
//}
//
//void hitLedTimer_tick() {
//        static uint32_t hitLedTimer = 0;
//        // State Action.
////      hitLedTimer_debugStatePrint();
//        switch(currentState) {
//        case init_st:                           // Standard init. Just reset the timer.
//                hitLedTimer = 0;        // Reset the timer.
//                break;
//        case waiting_for_timer_to_start_st:
//                hitLedTimer = 0;        // Waiting for the timer to start.
//                break;
//        case timer_running_st:
//                hitLedTimer++;          // Timer is running, increment the timer.
//                break;
//        default:
//                break;
//        }
//        // Next State.
//        switch(currentState) {
//        case init_st:
//                currentState = waiting_for_timer_to_start_st;   // Just jump to the wait state.
//                break;
//        case waiting_for_timer_to_start_st:
//                if (timerStartFlag) {                                                                   // Start the timer if asked and jump to the running state.
//                        currentState = timer_running_st;
//                        turnLedOn();
//                }
//                break;
//        case timer_running_st:                                                          // Running in this state just waiting for the timer to expire.
//                if (hitLedTimer >= HIT_LED_CLOCK_UP) {
//                        currentState = waiting_for_timer_to_start_st;   // Wait for the next lockout request if the timer has expired.
//                        timerStartFlag = false;                                                                                         // Also, reset the start flag so you won't keep looping.
//                        turnLedOff();
//                }
//                break;
//        default:
//                break;
//        }
//}


//
