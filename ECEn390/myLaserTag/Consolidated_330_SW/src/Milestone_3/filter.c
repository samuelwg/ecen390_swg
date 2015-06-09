/*


 * filter.c
 *
 *  Created on: Feb 10, 2015
 *      Author: samuelwg
 */
#include "filter.h"
#include "supportFiles/intervalTimer.h"
#include "supportFiles/utils.h"
#include "histogram.h"
#include <math.h>

#define IIR_NUM_COEF 11
#define IIR_DEN_COEF 11

// bit flags and macros
#define SET_BIT(var, bit) 		(var |= (1 << bit))
#define CLEAR_BIT(var, bit) 	(var &= ~(1 << bit))
#define TOGGLE_BIT(var, bit) 	(var ^= (1 << bit))
#define IS_SET(var, bit) 		(var & (1 << bit))

static volatile uint8_t flags;
#define INITIALIZED 0
#define FLAG2 1
#define FLAG3 2
#define FLAG4 3
#define FLAG5 4
#define FLAG6 5
#define FLAG7 6
#define FLAG8 7

#define IS_INITIALIZED	(flags & (1 << INITIALIZED))





double h[FIR_COEF_COUNT] = {0,-5.68521683853976e-06,-5.50709612255622e-06,3.18009072866910e-05,0.000131028798224967,0.000292295474029135,0.000476899452865295,0.000603806905166855,0.000562149034973060,0.000241643335583510,
		-0.000422676466485943,-0.00140157383046354,-0.00253432029300892,-0.00352102811734480,-0.00395948594403631,-0.00343196440243559,-0.00163285955264454,0.00148850489730424,0.00560922036946484,0.00998301126942438,
		0.0134972667774414,0.0148546731915080,0.0128664706544274,0.00680959653634157,-0.00322957120415137,-0.0161169785269475,-0.0296233247998513,-0.0406214879511540,-0.0455327810935330,-0.0409659495965289,-0.0244376724892752,
		0.00497226329214843,0.0461826202348593,0.0960098774128754,0.149463324942368,0.200413165356739,0.242526338069318,0.270300860959761,0.280000000000000,0.270300860959761,0.242526338069318,0.200413165356739,0.149463324942368,
		0.0960098774128754,0.0461826202348593,0.00497226329214843,-0.0244376724892752,-0.0409659495965289,-0.0455327810935330,-0.0406214879511540,-0.0296233247998513,-0.0161169785269475,-0.00322957120415137,0.00680959653634157,
		0.0128664706544274,0.0148546731915080,0.0134972667774414,0.00998301126942438,0.00560922036946484,0.00148850489730424,-0.00163285955264454,-0.00343196440243559,-0.00395948594403631,-0.00352102811734480,-0.00253432029300892,
		-0.00140157383046354,-0.000422676466485943,0.000241643335583510,0.000562149034973060,0.000603806905166855,0.000476899452865295,0.000292295474029135,0.000131028798224967,3.18009072866910e-05,-5.50709612255622e-06,-5.68521683853976e-06,0};  // h ideal coefficients for firFilter

double b[FILTER_IIR_FILTER_COUNT][IIR_NUM_COEF] ={5.53774282957553e-08,8.63013648449047e-09,-1.14197658483915e-06,-1.13939341744732e-07,3.72476608905632e-06,2.22127075317959e-07,-3.57641058827856e-06,-1.02923825030645e-07,1.01083279980851e-06,7.04199011677163e-09,-4.51866900890529e-08,
		5.53774282957553e-08,7.23128326703533e-09,-1.51430232867565e-06,-1.24345424035907e-07,5.51142373959059e-06,2.59290356889146e-07,-5.29183762641805e-06,-1.12322786872886e-07,1.34037456002849e-06,5.90055851255121e-09,-4.51866900890529e-08,
		5.53774282957553e-08,5.29977330815175e-09,-1.92045891839663e-06,-1.14216960138350e-07,7.74661272204120e-06,2.53891852708587e-07,-7.43789504033612e-06,-1.03172959224460e-07,1.69985966068909e-06,4.32449142886738e-09,-4.51866900890529e-08,
		5.53774282957553e-08,3.47929963325308e-09,-2.18864851188016e-06,-8.49905145770754e-08,9.38622737609855e-06,1.96376107219376e-07,-9.01211814761873e-06,-7.67722837480998e-08,1.93723156465299e-06,2.83902736355182e-09,-4.51866900890529e-08,
		5.53774282957553e-08,1.62145342178359e-09,-2.34766894095468e-06,-4.23732551896796e-08,1.04199111758415e-05,1.00075261016102e-07,-1.00045731129889e-05,-3.82758813293574e-08,2.07797894238780e-06,1.32306817992114e-09,-4.51866900890529e-08,
		5.53774282957553e-08,-9.18649103685022e-10,-2.37762567949022e-06,2.43020850428004e-08,1.06197636540926e-05,-5.76286213568604e-08,-1.01964541568667e-05,2.19521366528753e-08,2.10449334949200e-06,-7.49596245732232e-10,-4.51866900890529e-08,
		5.53774282957553e-08,-2.86852732558218e-09,-2.25370839501238e-06,7.20723495581997e-08,9.80360436950890e-06,-1.68041714541857e-07,-9.41284818525294e-06,6.51032115926036e-08,1.99481541095428e-06,-2.34065140368772e-09,-4.51866900890529e-08,
		5.53774282957553e-08,-5.60919071071116e-09,-1.86381711049113e-06,1.17477989027334e-07,7.41697861782909e-06,-2.58933882394913e-07,-7.12140662896146e-06,1.06118746439558e-07,1.64972656785784e-06,-4.57696881374957e-09,-4.51866900890529e-08,
		5.53774282957553e-08,-7.01212339673984e-09,-1.56668453298947e-06,1.24516067021810e-07,5.78292369896920e-06,-2.61913132733017e-07,-5.55251207546304e-06,1.12476818428180e-07,1.38673752068737e-06,-5.72172916919289e-09,-4.51866900890529e-08,
		5.53774282957553e-08,-8.39873793657332e-09,-1.20810002397999e-06,1.16840164128305e-07,4.02374244204715e-06,-2.30719169920443e-07,-3.86346853624904e-06,1.05543978096609e-07,1.06935798880496e-06,-6.85317429787978e-09,-4.51866900890529e-08}; //Numerator coefficients

double a[FILTER_IIR_FILTER_COUNT] [IIR_DEN_COEF] = {1, -7.35324580430287, 26.2311138204615, -58.8948353608113, 91.6454942368738, -102.950813933996, 84.4877620596799, -50.0539658433557, 20.5517728765454, -5.31097294132233, 0.665817942343443,
		1, -6.16136296785258, 19.7851949195999, -41.3987111276161, 61.9584760111218, -68.6830735503278, 57.1197667824781, -35.1846134167316, 15.5016326769979, -4.45012078676633, 0.665817942343443,
		1, -4.51563378075868, 12.7535768224597, -23.9850650844082, 34.3203944953708, -37.1408210080598, 31.6404936655824, -20.3851807573777, 9.99261871539083, -3.26147247906451, 0.665817942343443,
		1, -2.96451226189863, 8.11052326447354, -12.9909642998322, 18.7855295880269, -18.9847214598418, 17.3189483646002, -11.0413671813548, 6.35495715270760, -2.14115573260842, 0.665817942343443,
		1, -1.38154774168750, 5.35746950345114, -5.29281863547169, 10.5940859182420, -7.41220025848192, 9.76723361644873, -4.49857418036271, 4.19804086445021, -0.997839983664592, 0.665817942343443,
		1, 0.782728370453726, 4.83884109719401, 2.91743605078471, 9.13593943734179, 4.04893261920451, 8.42295986738527, 2.47965144847269, 3.79171459765273, 0.565335269147736, 0.665817942343443,
		1, 2.44410810411542, 6.98416888981087, 10.1594148986363, 15.3424058047474, 14.6042629915346, 14.1447277524264, 8.63480585414375, 5.47249995864334, 1.76528737813006, 0.665817942343443,
		1, 4.77927065617046, 13.7341926036701, 26.3235129451824, 37.8775637359811, 41.2001422477223, 34.9198244591175, 22.3725868010368, 10.7608950857838, 3.45188748067202, 0.665817942343443,
		1, 5.97462937451740, 18.8783238626162, 39.0594653446196, 58.1157954303204, 64.2830006158081, 53.5772486967442, 33.1965586586481, 14.7911325836648, 4.31525013401884, 0.665817942343443,
		1, 7.15608433360639, 25.0863465482443, 55.6758904825641, 86.0692694091044, 96.4769750294261, 79.3471383460919, 47.3183040890217, 19.6548898549831, 5.16857062487486, 0.665817942343443};//denominator coefficients

// queues
#define X_QUEUE_SIZE FIR_COEF_COUNT
#define Y_QUEUE_SIZE 11
#define Z_QUEUE_SIZE 10
#define P_QUEUE_SIZE_POWERS 10
#define P_QUEUE_SIZE 2000
#define CURRENT_POWER_VALUE 10

double y; //Passed through the FIR filter
double z;//Initialize for z Queue

static queue_t xQ;//Stores incoming inputs from ADC
static queue_t yQ;//Holds history of outputs from firFilter
static queue_t zQ[FILTER_IIR_FILTER_COUNT];// 10  of these to hold the IIR Filter outputs
static queue_t pQ[P_QUEUE_SIZE_POWERS];

static double currentPowerValue[CURRENT_POWER_VALUE] = {0};
static double old_powerValue[CURRENT_POWER_VALUE];
static bool firstPowerValue[CURRENT_POWER_VALUE] = {false};

static void initXQueue();
static void initYQueue();
static void initZQueues();
static void initPowerQueues();


// Must call this prior to using any filter functions.
// Will initialize queues and any other stuff you need to init before using your filters.
// Make sure to fill your queues with zeros after you initialize them.
void filter_init() {
	initXQueue();  // Call queue_init() on xQueue and fill it with zeros.
	initYQueue();  // Call queue_init() on yQueue and fill it with zeros.
	initZQueues(); // Call queue_init() on all of the zQueues and fill each z queue with zeros.
	initPowerQueues();

	SET_BIT(flags, INITIALIZED);
	return;
}
// Initialize all queues with zeros
static void initXQueue() {
	queue_init(&xQ, X_QUEUE_SIZE);

	for (int i=0; i<X_QUEUE_SIZE; i++) {
		queue_overwritePush(&xQ, 0.0);
	}
}


static void initYQueue() {
	queue_init(&yQ, Y_QUEUE_SIZE);

	for (int i=0; i<Y_QUEUE_SIZE; i++) {
		queue_overwritePush(&yQ, 0.0);
	}
}

static void initZQueues() {
	for (int i=0; i<FILTER_IIR_FILTER_COUNT; i++) {
		queue_init(&(zQ[i]), Z_QUEUE_SIZE);

		for (int j=0; j<Z_QUEUE_SIZE; j++) {
			queue_overwritePush(&(zQ[i]), 0.0);
		}
	}
}

static void initPowerQueues(){
	for (int i=0; i<P_QUEUE_SIZE_POWERS; i++) {
		queue_init(&(pQ[i]), P_QUEUE_SIZE);

		for (int j=0; j<P_QUEUE_SIZE; j++) {
			queue_overwritePush(&(pQ[i]), 0.0);
		}
	}

}

// Print out the contents of the xQueue for debugging purposes.
void filter_printXQueue() {
	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_printXQueue()\n\r");
		return;
	}

	queue_print(&xQ);
}

// Print out the contents of yQueue for debugging purposes.
void filter_printYQueue() {
	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_printYQueue()\n\r");
		return;
	}
	queue_print(&yQ);
}

// Print out the contents of the the specified zQueue for debugging purposes.
void filter_printZQueue(uint16_t filterNumber) {
	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_printZQueue()\n\r");
		return;
	}
	queue_print(zQ);
}

void filter_printPQueue(uint16_t filterNumber) {
	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_printZQueue()\n\r");
		return;
	}
	queue_print(pQ);
}

// Use this to copy an input into the input queue (x_queue).
void filter_addNewInput(double x) {
	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_addNewInput()\n\r");
		return;
	}

	queue_overwritePush(&xQ, x);
}

// Invokes the FIR filter. Returns the output from the FIR filter. Also adds the output to the y_queue for use by the IIR filter.
double filter_firFilter() {
	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_firFilter()\n\r");
		return 0.0;
	}
	 y = 0.0;
	  for (int i=0; i<FIR_COEF_COUNT; i++) {
	    double temp_x= queue_readElementAt(&xQ, i);
	    double temp_h= h[FIR_COEF_COUNT-1-i];  // iteratively adds the (b * input) products.
		y += temp_h*temp_x;
	  }
	  queue_overwritePush(&yQ, y);

	return y;
}

// Use this to invoke a single iir filter. Uses the y_queue and z_queues as input. Returns the IIR-filter output.
double filter_iirFilter(uint16_t filterNumber) {

	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_iirFilter()\n\r");
		return 0.0;
	}

	double temp_y;
	double temp_b;
	double bsum= 0.0;

	  for (int i=0; i<IIR_NUM_COEF; i++) {
          temp_y = queue_readElementAt(&yQ, i);
          temp_b = b[filterNumber][IIR_NUM_COEF-1-i];
           bsum += temp_y*temp_b;

	  }

//	  if(filterNumber == 0){
//		  printf("B : %E \r", bsum);
//	  }

	  double asum = 0.0;
	   for (int j=0; j<IIR_DEN_COEF-1; j++) {
            asum += queue_readElementAt(&zQ[filterNumber], j)*a[filterNumber][IIR_DEN_COEF-1-j];

	  }

//	   if(filterNumber == 0){
//		   printf("A : %E \r", asum);
//	   }

	  z = bsum - asum;
	  queue_overwritePush(&zQ[filterNumber], z);
	  old_powerValue[filterNumber] =  queue_readElementAt(&pQ[filterNumber], 0);
	  queue_overwritePush(&pQ[filterNumber], z*z);
	  filter_computePower(filterNumber, false, false);

//	   if(filterNumber == 0){
//	   	   printf("Z: %E \n\r",z);
//	   }
//
//	   if(0 == filterNumber){
//		   printf("P: %E \n\r", z*z);
//	   }
    return z;
}



// Use this to compute the power for values contained in a queue.
// If force == true, then recompute everything from scratch.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch, bool debugPrint) {

	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_computePower()\n\r");
		return 0.0;
	}
	if(forceComputeFromScratch){
		for(uint16_t i = 0; i < P_QUEUE_SIZE; i++){
			currentPowerValue[filterNumber] += queue_readElementAt(&pQ[filterNumber], i);
		}
	}else{
		currentPowerValue[filterNumber] += queue_readElementAt(&pQ[filterNumber], P_QUEUE_SIZE-1) -old_powerValue[filterNumber];
	}
	if(debugPrint){
		printf("I'm a debug print\n\r");
	}


	return 0.0;
//TODO[SAM]: WHAT AM I SUPPOSED TO RETURN? WHAT DOES THE DEBUG PRINT SUPPOSED TO DO?
}

double filter_getCurrentPowerValue(uint16_t filterNumber) {
	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_getCurrentPowerValue()\n\r");
		return 0.0;
	}
	return currentPowerValue[filterNumber];
}

// Takes an array of doulbes and returns the index of the max value in indexofMaxValue
// Stores computations in the global array normalizedArray
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t* indexOfMaxValue) {
	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_getNormalizedPowerValues()\n\r");
		return;
	}

	static uint16_t tempIndex = 0;
	tempIndex = 0;
	for(uint16_t i = 1; i <CURRENT_POWER_VALUE; i++){
//printf("A: %e\tB: %e\t i: %d\n\r",currentPowerValue[i], currentPowerValue[i-1], *indexOfMaxValue);
		if(currentPowerValue[i] > currentPowerValue[tempIndex]){
			tempIndex = i;
		}
	}

//printf("We think max is %d\n\r",tempIndex);

	//Index holds the position of the greatest value
	static double greatest_value;
	greatest_value = currentPowerValue[tempIndex];

	for(int j = 0; j < CURRENT_POWER_VALUE; j++){
		normalizedArray[j] = currentPowerValue[j]/greatest_value;
//printf("The normalized is array is: [%d]  %e \n\r",  j,normalizedArray[j]);
	}

	*indexOfMaxValue = tempIndex;
}


double mysignal[150]= {4000, 4095, 3049, 4095, 3412, 4000, 4095, 3049, 4095, 3412, 4000, 4095, 3049, 4095, 3412,
		   0, 588,0,951, 0, 0, 588, 0, 951, 0, 0, 588,  0, 951, 3412, 4000, 4095, 3049,  4095, 3412, 4000, 4095,
		   3049, 4095, 3412, 4000, 4095, 3049,  951,    0,    0, 588, 0,  951,    0,    0,  588,    0,  951,    0,    0,
		   588, 3049, 4095, 3412, 4000, 4095, 3049, 4095, 3412, 4000, 4095, 3049, 4095, 3412, 4000,  588,    0,
		   951,    0,    0,  588,    0,  951,    0,    0,  588,    0,  951,    0, 4000, 4095, 3049, 4095, 3412, 4000, 4095,
		   3049, 4095, 3412, 4000, 4095, 3049, 4095,  0,    0,  588,    0,  951,    0,    0,  588,    0,  951,    0,    0,
		   588,    0, 4095, 3412, 4000, 4095, 3049, 4095, 3412, 4000, 4095, 3049, 4095, 3412, 4000, 4095,    0,  951,    0,
		   0,  588,    0,  951,    0,   0,  588,    0,  951,    0,    0, 4095, 3049, 4095, 3412, 4000, 4095, 3049};

void filter_runTest() {
	filter_init();

	if(!IS_INITIALIZED) {
		printf("Queues are uninitialized. You must run filter_init() before you can run filter_runTest()\n\r");
		return;
	}

	static int count = 0;
	for(int i = 0; i<150; i++){
//		count++;
		filter_addNewInput(mysignal[i]);
		filter_firFilter();
//		if(10 == count){
//
//			count = 0;
//		}
		printf( " [%d] \r", i+1);
		for (int n = 0; n<FILTER_IIR_FILTER_COUNT;n++ ){
			filter_iirFilter(n);
			if(false == firstPowerValue[n]){
				filter_computePower(n, true, false);
				firstPowerValue[n] = true;
			}else{
				filter_computePower(n, false, false);

			}

		}

	}
	filter_printYQueue();
	filter_printZQueue(0);
	//	filter_printPQueue(0);

	 intervalTimer_start(1);
	 for(int i = 0; i < 10; i++){
			filter_computePower(0, true, false);
	 }

	intervalTimer_stop(1);

 double runningSeconds;
intervalTimer_getTotalDurationInSeconds(1, &runningSeconds);
printf("Total run time as measured by interval timer in seconds: %g.\n\r", runningSeconds);

intervalTimer_start(2);
	 for(int i = 0; i < 10; i++){
			filter_computePower(0, false, false);
	 }

	intervalTimer_stop(2);


intervalTimer_getTotalDurationInSeconds(2, &runningSeconds);
printf("Total run time as measured by interval timer in seconds: %g.\n\r", runningSeconds);
printf("Filter power value for 0: %E\n\r", filter_getCurrentPowerValue(0));

uint16_t maxIndex = 0;
filter_getNormalizedPowerValues(currentPowerValue,&maxIndex);



printf("Max Value is: %d \n\r", maxIndex);

}


/*=============================================================================
 * ================= Test Routines Start Here =================================
 ==============================================================================*/

/* ===================== Test Variable Definitions ==================== */
static uint16_t firDecimationCount = 0;  // Used to keep track of samples relative to the decimation factor.

/* ================================== #defines ================================ */
#define FILTER_IIR_POWER_TEST_PERIOD_COUNT FILTER_FREQUENCY_COUNT
#define FILTER_FREQUENCY_COUNT FILTER_IIR_FILTER_COUNT
#define FILTER_SAMPLE_FREQUENCY_IN_KHZ 100

// NOTE: YOU MUST DEFINE THIS PROPERLY FOR THE TEST TO WORK PROPERLY.
// If you used a leading 1 in the A-coefficient array for your IIR, this should be defined to be a 1.
// If you did not use a leading 1 in the A-coefficient array for your IIR, this should be defined to be 0.
#define FILTER_TEST_USED_LEADING_1_IN_IIR_A_COEFFICIENT_ARRAY 1

/* ===================== Simple Accessor Functions ==================== */

// Returns the array of FIR coefficients.
double* filterTest_getFirCoefficientArray() {
  return h;
}

// Returns the number of FIR coefficients.
uint32_t filterTest_getFirCoefficientCount() {
  return FIR_COEF_COUNT;
}

// Returns the array of coefficients for a particular filter number.
double* filterTest_getIirACoefficientArray(uint16_t filterNumber) {
  return  a[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filterTest_getIirACoefficientCount() {
  return IIR_DEN_COEF;
}

// Returns the array of coefficients for a particular filter number.
double* filterTest_getIirBCoefficientArray(uint16_t filterNumber) {
  return  b[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filterTest_getIirBCoefficientCount() {
  return IIR_NUM_COEF;
}

// Returns the size of the yQueue.
uint32_t filterTest_getYQueueSize() {
  return Y_QUEUE_SIZE;
}

// Returns the decimation value.
uint16_t filterTest_getDecimationValue() {
  return FILTER_FIR_DECIMATION_FACTOR;
}

// Returns the starting index for the A-coefficient array for the IIR-filter.
// Some students use the leading 1, others did not.
uint16_t filterTest_getIirACoefficientArrayStartingIndex() {
  if (FILTER_TEST_USED_LEADING_1_IN_IIR_A_COEFFICIENT_ARRAY)
    return 1;
  else
    return 0;
}

/*==========================Helper Test Functions ============================ */

// Returns true if the values are within TEST_FILTER_FLOATING_POINT_EPSILON of each other.
//#define TEST_FILTER_FLOATING_POINT_EPSILON 1.0E-15L
static const double eps = 1.0E-15;
bool filterTest_floatingPointEqual(double a, double b) {
  //	return abs(a-b) < TEST_FILTER_FLOATING_POINT_EPSILON;
  return fabs(a-b) < eps;
}

// Fills the queue with the fillValue, overwriting all previous contents.
void filterTest_fillQueue(queue_t* q, double fillValue) {
  for (queue_size_t i=0; i<q->size; i++) {
    queue_overwritePush(q, fillValue);
  }
}

queue_data_t filterTest_filter_readMostRecentValueFromQueue(queue_t* q) {
  return queue_readElementAt(q, q->elementCount-1);
}

/* ============================ Major Test Functions ============================= */

// Invokes FIR-filter according to the decimation factor.
// Returns true if it invokes filter_firFilter().
bool filter_decimatingFirFilter() {
  if (firDecimationCount == filterTest_getDecimationValue()-1) {  // Time to run the FIR filter?
    filter_firFilter();      // Run the FIR filter.
    firDecimationCount = 0;  // Reset the decimation count.
    return true;             // Return true because you ran the FIR filter.
  }
  firDecimationCount++;  // Increment the decimation count each time you call this filter.
  return false;          // Didn't run the FIR filter.
}

// Pushes a single 1.0 through the xQueue. Golden output data are just the FIR coefficients in reverse order.
// If this test passes, you are multiplying the coefficient with the correct element of xQueue.
bool filterTest_runFirAlignmentTest(bool printMessageFlag) {
  bool success = true;	 					// Be optimistic.
  filterTest_fillQueue(&xQ, 0.0);	// zero-out the xQueue.
  filter_addNewInput(1.0);				// Place a single 1.0 in the xQueue.
  for (uint32_t i=0; i<filterTest_getFirCoefficientCount(); i++) {	// Push the single 1.0 through the queue.
    filter_firFilter();								// Run the FIR filter.
    double firValue = filterTest_filter_readMostRecentValueFromQueue(&yQ);
    double firGoldenOutput =  filterTest_getFirCoefficientArray()[filterTest_getFirCoefficientCount()-i-1];	// Golden output is simply the FIR coefficient.
    if (!filterTest_floatingPointEqual(firValue, firGoldenOutput)) {	// If the output from the FIR filter does not match the golden value, print an error.
      success = false;									// The test failed.
      printf("filter_runAlignmentTest: Output from FIR Filter(%le) does not match test-data(%le).\n\r", firValue, firGoldenOutput);
    }
    filter_addNewInput(0.0);	// Shift the 1.0 value over one position in the queue.
  }
  // Print informational messages.
  if (printMessageFlag) {
    printf("filter_runFirAlignmentTest ");
    if (success)
      printf("passed.\n\r");
    else
      printf("failed.\n\r");
  }
  return success;	// Return the success of failure of this test.
}

// Fills the xQueue with 1.0. Invokes filter_firFilter() and compares the output with the
// sum of the coefficients.
bool filterTest_runFirArithmeticTest(bool printMessageFlag) {
  bool success = true;							// Be optimistic.
  filterTest_fillQueue(&xQ, 0.0);  // zero-out the xQueue.
  double firGoldenOutput = 0.0;			          // You will compute the golden output by accumulating the FIR coefficients.
  for (uint32_t i=0; i<filterTest_getFirCoefficientCount(); i++) {	// Loop enough times to go through the coefficients.
    firGoldenOutput += filterTest_getFirCoefficientArray()[i];
  }
  filterTest_fillQueue(&xQ, 1.0);
  filter_firFilter();
  double firValue = filterTest_filter_readMostRecentValueFromQueue(&yQ);
  if (!filterTest_floatingPointEqual(firValue, firGoldenOutput)) {
    success = false;									// Test failed.
    printf("filter_runArithmeticTest: Output from FIR Filter(%le) does not match test-data(%le).\n\r", firValue, firGoldenOutput);
  }
  // Print informational messages.
  if (printMessageFlag) {
    printf("filter_runFirArithmeticTest ");
    if (success)
      printf("passed.\n\r");
    else
      printf("failed.\n\r");
  }
  return success;	// Return the success or failure of the test.
}

// This test checks to see that the B coefficients are multiplied with the correct values of the yQueue.
// If it passes, the coefficients are properly aligned with the data in yQueue.
// This test only checks the coefficients for filterNumber (frequency).
bool filterTest_runIirBAlignmentTest(uint16_t filterNumber, bool printMessageFlag) {
  bool success = true;						// Be optimistic.
  filterTest_fillQueue(&yQ, 0.0);	// zero-out the yQueue.
  filterTest_fillQueue(&(zQ[filterNumber]), 0.0);	// zero out the zQueue for filterNumber.
  queue_overwritePush(&yQ, 1.0);							// Place a single 1.0 in the yQueue.
  for (uint32_t i=0; i<filterTest_getIirBCoefficientCount(); i++) {
    filter_iirFilter(filterNumber);										// Run the IIR filter.
    double iirValue = filterTest_filter_readMostRecentValueFromQueue(&(zQ[filterNumber]));										// Run the IIR filter.
    double iirGoldenOutput =  filterTest_getIirBCoefficientArray(filterNumber)[i];	// Golden output is simply the coefficient.
    if (!filterTest_floatingPointEqual(iirValue, iirGoldenOutput)) {	// Make sure they IIR output matches the golden output.
      success = false;									// Note test failure and print message.
      printf("filter_runIirBlignmentTest: Output from IIR Filter[%d](%le) does not match test-data(%le) at index(%ld).\n\r", filterNumber, iirValue, iirGoldenOutput, i);
    }
    filterTest_fillQueue(&(zQ[filterNumber]), 0.0);	// zero out the zQueue for filterNumber so the A-summation is always 0.
    queue_overwritePush(&yQ, 0.0);							// Shift the 1.0 over one position in the yQueue.
  }
  // Print informational messages.
  if (printMessageFlag) {
    printf("filter_runIirBAlignmentTest ");
    if (success)
      printf("passed.\n\r");
    else
      printf("failed.\n\r");
  }
  return success;	// Return the success or failure of the test.
}

// Checks to see that the A-coefficients are properly aligned with the zQueue.
// This test checks the A-coefficients for a specific filter-number (frequency).
// It is a little more work to create a sliding 1 in the zQueue because the IIR updates the zQueue.
// Thus as you move across coefficients, the zQueue is cleared out. 1.0 is added and shifted over the correct number of spaces.
bool filterTest_runIirAAlignmentTest(uint16_t filterNumber, bool printMessageFlag) {
  bool success = true;						// Be optimistic.
  filterTest_fillQueue(&yQ, 0.0);	// zero-out the yQueue so the B-summation is always 0.
  uint16_t startingIndex = filterTest_getIirACoefficientArrayStartingIndex();  // Varies according to student.
  for (uint32_t i=0; i<filterTest_getIirACoefficientCount()-startingIndex; i++) {	// Loop through all of the A-coefficients.
    filterTest_fillQueue(&(zQ[filterNumber]), 0.0);				// zero out the zQueue for filterNumber.
    queue_overwritePush(&(zQ[filterNumber]), 1.0);		// Add a single 1.0 to the queue.
    for (uint32_t j=0; j<i; j++) {												// Move over the 1.0 an additional position each time through the loop.
      queue_overwritePush(&(zQ[filterNumber]), 0.0);	// Move the 1.0 over by writing the correct number of zeros.
    }
    filter_iirFilter(filterNumber);										// Run the IIR filter.
    double iirValue = filterTest_filter_readMostRecentValueFromQueue(&(zQ[filterNumber]));										// Run the IIR filter.
    double iirGoldenOutput =  filterTest_getIirACoefficientArray(filterNumber)[i+startingIndex];	// Golden output is simply the coefficient.
    if (!filterTest_floatingPointEqual(iirValue, -iirGoldenOutput)) {							// Check to see that the IIR-output matches the correct coefficient value.
      success = false;																									// Note the failure of the test and print an info message.
      printf("filter_runIirAlignmentTest: Output from IIR Filter[%d](%le) does not match test-data(%le) at index(%ld).\n\r", filterNumber, iirValue, iirGoldenOutput, i);
    }
  }
  // Print informational messages.
  if (printMessageFlag) {
    printf("filter_runIirAAlignmentTest ");
    if (success)
      printf("passed.\n\r");
    else
      printf("failed.\n\r");
  }
  return success;	// Return the failure or success of the test.
}

// Normalizes the values in the array argument.
void filterTest_normalizeArrayValues(double* array, uint16_t size) {
  // Find the maximum value
  uint16_t maxIndex = 0;
  // Find the index of the max value in the array.
  for (int i=0; i<size; i++) {
    if (array[i] > array[maxIndex])
      maxIndex = i;
  }
  double maxPowerValue = array[maxIndex];
  // Normalize everything between 0.0 and 1.0, based upon the max value.
  for (int i=0; i<size; i++)
    array[i] = array[i] / maxPowerValue;
}

#define FILTER_FIR_POWER_TEST_PERIOD_COUNT 22
#define FILTER_TEST_HISTOGRAM_BAR_COUNT FILTER_FIR_POWER_TEST_PERIOD_COUNT
// Everything is defined assuming a 100 kHz sample rate.
static const uint16_t filter_testPeriodTickCounts[FILTER_FIR_POWER_TEST_PERIOD_COUNT] = {90, 72, 58, 50, 44, 38, 34, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2};
#define MAX_BUF 10
#define FILTER_TEST_PULSE_WIDTH_LENGTH 20000

// Plots the frequency response of the FIR filter on the TFT.
// Everything is defined assuming a 100 kHz sample rate.
// Frequencies run from 1.1 kHz to 50 kHz.
void filterTest_runFirPowerTest(bool printMessageFlag) {
  if (printMessageFlag) {
    // Tells you that this function is plotting the frequency response for the FIR filter for a set of frequencies.
    printf("running filter_runFirPowerTest() - plotting power values (frequency response) for frequencies %1.2lf kHz to %1.2lf kHz for FIR filter to TFT display.\n\r",
	   ((double) ((FILTER_SAMPLE_FREQUENCY_IN_KHZ))/filter_testPeriodTickCounts[0]),
	   ((double) ((FILTER_SAMPLE_FREQUENCY_IN_KHZ))/filter_testPeriodTickCounts[FILTER_FIR_POWER_TEST_PERIOD_COUNT-1]));
  }
  firDecimationCount = 0;
  double testPeriodPowerValue[FILTER_FIR_POWER_TEST_PERIOD_COUNT];	// Computed power values will go here.
  uint16_t freqCount=0;	// Used to print info message.
  // Simulate running everything at 100 kHz. Simply add either 1.0 or -1.0 to xQueue based upon the
  // the frequency you are simulating.
  // Iterate over all of the test-periods.
  for (uint16_t testPeriodIndex=0; testPeriodIndex<FILTER_FIR_POWER_TEST_PERIOD_COUNT; testPeriodIndex++) {
    uint16_t currentPeriodTickCount = filter_testPeriodTickCounts[testPeriodIndex];
    double power = 0.0;						// Temp variable to compute power.
    uint32_t totalTickCount = 0;	// Keep track of where you are in the period.
    while (totalTickCount < FILTER_TEST_PULSE_WIDTH_LENGTH) {	// Keep going until you have completed the entire pulse-width.
      for (uint16_t freqTick = 0; freqTick < currentPeriodTickCount; freqTick++) {	// This loop completes a single period.
	if (freqTick < currentPeriodTickCount/2)	// The first half of the period is -1.0.
	  filter_addNewInput(-1.0);		// Add the simulated value to the FIR filter.
	else
	  filter_addNewInput(+1.0);		// In the second half of the period, add +1.0 to the FIR filter.
	if (filter_decimatingFirFilter()) {	// Tells us if we ran the the FIR filter (we are decimating so the FIR filter only runs every 10 input values).
	  double firOutput = filterTest_filter_readMostRecentValueFromQueue(&yQ);	// Get the output from the yQueue.
	  power += firOutput * firOutput;	// Compute the power so far.
	  totalTickCount++;
	}
      }
    }
    testPeriodPowerValue[testPeriodIndex] = power;	// All done computing power for this period, move on to the next one.
    //		printf("Frequency[%d](%5.4lf) Power:%5.2le\n\r", freqCount, (double) (FILTER_SAMPLE_FREQUENCY_IN_KHZ/currentPeriodTickCount), power);
    freqCount++;
  }
  // Now we start plotting the results.
  filterTest_normalizeArrayValues(testPeriodPowerValue, FILTER_FIR_POWER_TEST_PERIOD_COUNT);	// Normalize the values.
  histogram_init(FILTER_TEST_HISTOGRAM_BAR_COUNT);	// Init the histogram.
  // Set labels and colors (blue) for the standard frequencies 0-9.
  for (int i=0; i<FILTER_FREQUENCY_COUNT; i++) {
    histogram_setBarColor(i, DISPLAY_BLUE);		// Sets the color of the bar.
    char tempLabel[MAX_BUF];									// Temp variable for label generation.
    snprintf(tempLabel, MAX_BUF, "%d", i);		// Create the label that represents one of the user frequencies.
    histogram_setBarLabel(i, tempLabel);			// Finally, set the label.
  }
  // Set the colors for the other nonstandard frequencies to be red so that the stand out.
  for (int i=10; i<FILTER_FIR_POWER_TEST_PERIOD_COUNT; i++) {
    histogram_setBarColor(i, DISPLAY_RED);
    char tempLabel[MAX_BUF];	// Used to create labels.
    // Create three kinds of labels.
    // 1. Just label the first set of defined frequencies 0-9.
    // 2. This is the start of the frequencies outside the actual transmitted frequencies.
    // The bounds are printed at the start and end of this range, using the labels so that they display OK in the limited space.
    // 3. The lower bound for out-of-bound testing frequencies.
    if (i == 10) {
      snprintf(tempLabel, MAX_BUF, "%dkHz",  FILTER_SAMPLE_FREQUENCY_IN_KHZ/filter_testPeriodTickCounts[i]);
      histogram_setBarLabel(i, tempLabel);
      // 4. Print the upper bound for out-of-bound testing frequencies further to the left to make readable.
    } else if (i == FILTER_FIR_POWER_TEST_PERIOD_COUNT-4) {
      snprintf(tempLabel, MAX_BUF, "%dkHz",  FILTER_SAMPLE_FREQUENCY_IN_KHZ/filter_testPeriodTickCounts[FILTER_FIR_POWER_TEST_PERIOD_COUNT-1]);
      histogram_setBarLabel(i, tempLabel);
      // Print a "-" for readability.
    } else if (i == FILTER_FIR_POWER_TEST_PERIOD_COUNT-7) {
      histogram_setBarLabel(i, "-");
      // Print blank space everywhere else.
    } else {
      histogram_setBarLabel(i, "");

    }
  }
  // Set the actual histogram-bar data to be power values..
  for (uint16_t barIndex=0; barIndex<FILTER_TEST_HISTOGRAM_BAR_COUNT; barIndex++) {
    histogram_setBarData(barIndex, testPeriodPowerValue[barIndex] * HISTOGRAM_MAX_BAR_DATA_IN_PIXELS, "");
  }
  histogram_redrawBottomLabels();  // Need to redraw the bottom labels because I changed the colors.
  histogram_updateDisplay();	   // Redraw the histogram.
}

// Plots frequency response for the selected filterNumber against the 10 standard frequencies.
void filterTest_runIirPowerTest(uint16_t filterNumber, bool printMessageFlag) {
  if (printMessageFlag) {
    printf("running filter_runFirPowerTest(%d) - plotting power for all player frequencies for IIR filter(%d) to TFT display.\n\r", filterNumber, filterNumber);
  }
  firDecimationCount = 0;
  double testPeriodPowerValue[FILTER_IIR_POWER_TEST_PERIOD_COUNT];	// Keep track of power values here.
  uint16_t freqCount=0;
  // Simulate running everything at 100 kHz.
  for (uint16_t testPeriodIndex=0; testPeriodIndex<FILTER_FREQUENCY_COUNT; testPeriodIndex++) {  // Only use the first 10 standard frequencies.
    uint16_t currentPeriodTickCount = filter_testPeriodTickCounts[testPeriodIndex];		// You will be generating a frequency with this period.
    double power = 0.0;						// Used to compute power.
    uint32_t totalTickCount = 0;	// Keep track of where you are in the given period.
    while (totalTickCount < FILTER_TEST_PULSE_WIDTH_LENGTH) {		// Generate a pulse-width of periods.
      double iirOutput = 0.0;		// Output from the IIR filter.
      for (uint16_t freqTick = 0; freqTick < currentPeriodTickCount; freqTick++) {	// This loop generates one period of the frequency.
	if (freqTick < currentPeriodTickCount/2)	// First half of period is -1.0.
	  filter_addNewInput(-1.0);
	else
	  filter_addNewInput(+1.0);								// Second half of period is +1.0
	if (filter_decimatingFirFilter()) {				// If you actually ran the FIR-filter, run the IIR-filter.
	  filter_iirFilter(filterNumber);
	  iirOutput = filterTest_filter_readMostRecentValueFromQueue(&(zQ[filterNumber]));
	}
	power += iirOutput * iirOutput;	// Compute the power at the output.
	totalTickCount++;
      }
    }
    testPeriodPowerValue[testPeriodIndex] = power;
    //		printf("Frequency[%d](%5.4lf) Power:%5.2le\n\r", freqCount, (double) (FILTER_SAMPLE_FREQUENCY_IN_KHZ/currentPeriodTickCount), power);
    freqCount++;
  }
  // Make a copy of the power values that you will normalize.
  double normalizedPowerValue[FILTER_IIR_POWER_TEST_PERIOD_COUNT];
  for (int i=0; i<FILTER_IIR_POWER_TEST_PERIOD_COUNT; i++) {
    normalizedPowerValue[i] = testPeriodPowerValue[i];
  }
  filterTest_normalizeArrayValues(normalizedPowerValue, FILTER_IIR_POWER_TEST_PERIOD_COUNT);
  histogram_init(FILTER_IIR_POWER_TEST_PERIOD_COUNT);
  // Set labels and colors (red) for the standard frequencies 0-9.
  // Default is red but will change the color for the desired frequency to be blue.
  for (int i=0; i<FILTER_IIR_POWER_TEST_PERIOD_COUNT; i++) {
    histogram_setBarColor(i, DISPLAY_RED);
    char tempLabel[MAX_BUF];
    snprintf(tempLabel, MAX_BUF, "%d", i);
    histogram_setBarLabel(i, tempLabel);
  }
  histogram_setBarColor(filterNumber, DISPLAY_BLUE);	// Desired frequency drawn in blue.
  for (uint16_t barIndex=0; barIndex<FILTER_IIR_POWER_TEST_PERIOD_COUNT; barIndex++) {
    char label[HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS];	// Get a buffer for the label.
    // Create the top-label, based upon the actual power value.
    if (snprintf(label, HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS, "%0.0e", testPeriodPowerValue[barIndex]) == -1)
      printf("Error: snprintf encountered an error during conversion.\n\r");
    // Pull out the 'e' from the exponent to make better use of your characters.
    trimLabel(label);
    histogram_setBarData(barIndex, normalizedPowerValue[barIndex] * HISTOGRAM_MAX_BAR_DATA_IN_PIXELS, label);	// No top-label.
  }
  histogram_redrawBottomLabels();  // Need to redraw the bottom labels because I changed the colors.
  histogram_updateDisplay();	   // Redraw the histogram.
}

// 1. Tests the FIR filter in isolation using test-data, with no decimation.
// 2. Tests the FIR-filter using actual data, with decimation, using data generated by the transmitter code but not going
// through the ADC. The data generated by the transmitter are scaled between -1.0 and 1.0 and placed directly in the xQueue.
// The power in the FIR output is measured and compared to a threshold.
// 3. Tests the IIR in isolation using test-data.
// 4. Tests the
#define TEN_SECONDS 10000
#define TWO_SECONDS 2000
bool filter_runTest_hutch() {
  bool success = true;	// Be optimistic.
  filter_init();
  success &= filterTest_runFirAlignmentTest(true);
  success &= filterTest_runFirArithmeticTest(true);
  success &= filterTest_runIirAAlignmentTest(0, true);
  success &= filterTest_runIirBAlignmentTest(0, true);
  filterTest_runFirPowerTest(true);
  utils_msDelay(TEN_SECONDS);
  for (int i=0; i<FILTER_IIR_FILTER_COUNT; i++) {
    filterTest_runIirPowerTest(i, true);
    utils_msDelay(TWO_SECONDS);
  }
  return success;
}

