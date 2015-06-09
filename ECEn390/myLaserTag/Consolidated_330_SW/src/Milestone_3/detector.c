/*
 * detector.c
 *
 *  Created on: Mar 10, 2015
 *      Author: samuelwg
 */

#include "detector.h"
#include "filter.h"
#include "isr.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"
#include "supportFiles/interrupts.h"
#include "transmitter.h"
#include "supportFiles/switches.h"

#define ADC_RESOLUTION 2048
#define FILTER_COUNT 10
#define NUMBER_OF_PLAYERS 10
#define COPIED_POWER_VALUE 10
#define MEDIAN_VALUE 4
#define FUDGE_FACTOR 45

static volatile bool hit_detectedFlag = false;//raise flag when hit is detected

static double power[COPIED_POWER_VALUE] = {0};// Holds value from get power function
static double sorted_power[COPIED_POWER_VALUE] = {0};// Holds the sorted values

static double player_hit[COPIED_POWER_VALUE] = {0};// increments at the index of the frequency of the IIR filter

//static double test_Power[COPIED_POWER_VALUE] =  {1, 2, 1.5, 1.6, 1.1, 1.3, 1.9, 1.2, 2.1, 20};
//static double test_sortedpowerValue[COPIED_POWER_VALUE] = {1, 2, 1.5, 1.6, 1.1, 1.3, 1.9, 1.2, 2.1, 20};// {500, 600, 550, 100, 200, 300, 400, 150, 250, 350}; //{500, 600, 550, 100, 200, 300, 400, 150, 250, 350}



// Always have to init things.
void detector_init(){
	//filter_init();

}

// Used to grab debug values during debugging.
queue_t* detector_getDebugQueue(){


}

// Runs the entire detector: decimating fir-filter, iir-filters, power-computation, hit-detection.
void detector(){

	static uint8_t i = 0;

	static uint32_t elementCount = isr_adcBufferElementCount();

// Run that for loop elementcount number of times
	for(uint8_t j = 0; j < elementCount; j++){
		interrupts_disableArmInts();
		uint32_t rawAdcValue = isr_removeDataFromAdcBuffer();
		interrupts_enableArmInts();

       // should be scaled from -1 to 1
		double scaledAdcValue = (rawAdcValue*1.0/ADC_RESOLUTION)-1;
		filter_addNewInput(scaledAdcValue);

		// Perform the decimating FIR filter, IIR Filter and power computation for the 10 channels
		if(i < 10-1){
			i++;
		}else{
			filter_firFilter();
			for(uint8_t n = 0; n < FILTER_COUNT; n++){
				filter_iirFilter(n);

			}
            // Run the hit detection algorithm if lockout timer is not running
            if(!lockoutTimer_running()){
            	hit_detectedFlag = false;
                detector_sort_ascend();
                detector_median_threshold();
//                detector_hitDetect();

                // Once the hit is detected, run start our lockout timer, hitLed timer and increment the hit array
                if(hit_detectedFlag){
                    lockoutTimer_start();
                    hitLedTimer_start();
                }
            }
				i = 0;
		}


	}
}

// Invoke to determine if a hit has occurred.
bool detector_hitDetected(){
	return hit_detectedFlag;

}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(){
	hit_detectedFlag = false;

}

// Get the current hit counts.
void detector_getHitCounts(detector_hitCount_t hitArray[]){
	//Copy array over to the detector hit array
	for(uint8_t i = 0; i < NUMBER_OF_PLAYERS; i++){
		hitArray[i] = player_hit[i];
	}
}

void detector_runTest(){
	detector_isolatedTest();

}

// Sorts the power values in ascending order
void detector_sort_ascend(){
    //copy over the power values from the filter.
    // One will be exact copy. Other will be me sorted
    for(uint8_t j = 0; j < COPIED_POWER_VALUE; j++){
        power[j] = filter_getCurrentPowerValue(j);
        sorted_power[j] = filter_getCurrentPowerValue(j);
    }

	//Sorting algorithm
	for(uint32_t t = 1; t < 10; t++){
		double x = sorted_power[t];
		int s = t;
		while(s>0 && sorted_power[s-1]>x){
			sorted_power[s] = sorted_power[s-1];
			s = s - 1;
		}
		sorted_power[s] = x;
	}
}

// Grabs median value from sorted array and computes threshold
void detector_median_threshold(){
	double tempMax = power[0];
	static double threshold = 0;
	uint16_t indexofMax = 0;

	for(uint8_t i = 0; i < 10; i++){
		if(tempMax < power[i]){
			indexofMax = i;
			tempMax = power[i];
		}
	}

	//Find the threshold from median value
	threshold = sorted_power[MEDIAN_VALUE]*FUDGE_FACTOR;
	if(sorted_power[NUMBER_OF_PLAYERS - 1] > threshold && (indexofMax != switches_read())){
		hit_detectedFlag = true;
		player_hit[indexofMax] = player_hit[indexofMax] + 1;
	}



}

//Detects a hit at then finds who it was
void detector_hitDetect(){
	//Find the person who was the shooter from copiedPowerValue array
	// Increment if they keep shooting.
//
//		for(int k = 0 ; k < 10 ;k++){
//			if(sorted_power[9] == power[k]){
//
//					hit_detectedFlag = true;
//					player_hit[k] = player_hit[k] + 1;
//				}
//		}



}

void detector_isolatedTest(){
    //use test arrays for the isolated test
	detector_sort_ascend();

//	printf("largest value is: %lf\n\r",test_sortedpowerValue[9]);

	detector_median_threshold();

	if(hit_detectedFlag){
		lockoutTimer_start();
		hitLedTimer_start();
		detector_hitDetect();
	}

	if(detector_hitDetected()){
		printf("hit detected\n\r");
	}else{
		printf("hit not detected\n\r");
	}
}

