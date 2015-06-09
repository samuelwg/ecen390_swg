/*
 * buttons.h
 *
 *  Created on: Sep 3, 2014
 *      Author: hutch
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>

#define BUTTONS_INIT_STATUS_OK 1
#define BUTTONS_INIT_STATUS_FAIL 0
#define BUTTONS_BTN0_MASK 0x1
#define BUTTONS_BTN1_MASK 0x2
#define BUTTONS_BTN2_MASK 0x4
#define BUTTONS_BTN3_MASK 0x8


// Provides functions to read the push buttons (BTN3, BTN2, BTN1, BTN0)

// Initializes the button driver software and hardware.
int buttons_init();

// Returns the current value of all 4 buttons as the lower 4 bits of the returned value.
// bit3 = BTN3, bit2 = BTN2, bit1 = BTN1, bit0 = BTN0.
int32_t buttons_read();

// Runs a test of the buttons. As you push the buttons, messages will be written to the LCD
// panel. The test will run for about 10 seconds.
void buttons_runTest();

#endif /* BUTTONS_H_ */
