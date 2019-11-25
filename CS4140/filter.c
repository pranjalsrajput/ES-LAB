


#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "filter.h"

#define FILTER_SCALE  14
#define FILTER_NZEROS  1
#define FILTER_COUNT 5

static int32_t FILTER_GainFixed50HzCut = 0;


static int32_t xv[(FILTER_NZEROS + 1) * FILTER_COUNT];
static int32_t yv[(FILTER_NZEROS + 1) * FILTER_COUNT];

static int32_t b0_50HzCut;


static int32_t FILTER_FloatToFixed(float x);

static int32_t FILTER_FixMult(int32_t x, int32_t y);

/**
 * FILTER_FloatToFixed convert a float variable to a int32_t fixed-point arithmetic variable.
 * @author Miguel Pérez Ávila.
 * @param  float x. Float value to convert to fixed-point arithmetic.
 * @return int32_t. The fixed-point result.
 * */
static int32_t FILTER_FloatToFixed(float x) {

    int32_t y = 0;

    y = x * (1 << FILTER_SCALE);

    return y;
}


/**
 * FILTER_FixMult compute the fixed-point arithmetic multiplication of two pixed-point arithmetic variables. It scales the result to the fixed-point arithmetic.
 * @author Miguel Pérez Ávila.
 * @param int32_t x. A fixed-point arithmetic variable.
 * @param int32_t y. A fixed-point arithmetic variable.
 * @return int32_t. The fixed-point result.
 * */
static int32_t FILTER_FixMult(int32_t x, int32_t y) {

    int32_t z;

    z = x * y;
    z = (z >> FILTER_SCALE);

    return z;

}

/**
 * FILTER_50HzCutInit It initializes the variables from the 50Hz low pass butterworth filter. Adjust the gain to a fized-point value and the coeficients of the filter.
 * Initialize the number of windows that are filtered to zero as number of filters that have been defined in FILTER_COUNT.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
void FILTER_50HzCutInit(void) {
    FILTER_GainFixed50HzCut = FILTER_FloatToFixed(7.313751515);
    b0_50HzCut = FILTER_FloatToFixed(0.7265425280);

    int i = 0;
    for (i = 0; i < (FILTER_NZEROS + 1) * FILTER_COUNT; i++) {
        xv[i] = 0;
        yv[i] = 0;
    }
}


/**
 * FILTER_50HzCutRun It runs the 50Hz low pass filter. Update the window filter with the new measure and give back the filter variable.
 * With index you can select which window filter are you going to use and newMeas is the new  measure that are you going to apply to the window filter.
 * This function allows you to use the same filter for as many raw data as filters have been defined in FILTER_COUNT.
 * @author Miguel Pérez Ávila.
 * @param  uint8_t index. Which window filter are you going to use.
 * @param  int16_t newMeas. New measure read.
 * @return int32_t. The filtered result.
 * */
int32_t FILTER_50HzCutRun(uint8_t index, int16_t newMeas) {
    xv[0 + 2 * index] = xv[1 + 2 * index];
    xv[1 + 2 * index] = ((newMeas << FILTER_SCALE)) / FILTER_GainFixed50HzCut;
    yv[0 + 2 * index] = yv[1 + 2 * index];
    yv[1 + 2 * index] = (xv[0 + 2 * index] + xv[1 + 2 * index]) + FILTER_FixMult(b0_50HzCut, yv[0 + 2 * index]);
    return yv[1 + 2 * index];
}

