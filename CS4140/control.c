/*------------------------------------------------------------------
 *  control.c -- here you can implement your control algorithm
 *		 and any motor clipping or whatever else
 *		 remember! motor input =  0-1000 : 125-250 us (OneShot125)
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"
#include "control_params.h"
#include "comm_mcu.h"
#include "kalman.h"
#include "calibration.h"

#define MAX_SPEED 800 // TODO: parameter to be changed

/**
 * set motor values from ae
 */
void update_motors(void) {
    for (uint8_t i = 0; i < 4; i++) {
        if (ae[i] > MAX_SPEED) {
            motor[i] = MAX_SPEED;
        } else if (ae[i] < 0) {
            motor[i] = 0;
        } else {
            motor[i] = ae[i];
        }
    }
}

/**
 * basic int32 square root function
 * by Beni Kovács
 * @param number the number
 * @return the int square root of the number
 */
int32_t square_root(int32_t number) {
    if (number <= 0) {
        return 0;
    }
    int32_t bottom = 1;
    int32_t top = number / 2;
    while (bottom + 1 < top) {
        int32_t half = (bottom + top) / 2;
        if (half * half == number) {
            return half;
        } else if (half * half < number) {
            bottom = half;
        } else {
            top = half;
        }
    }
    return bottom;
}

/**
 * compute ae values to be sent to the motor, based on the input parameters
 * by Pranjal Singh Rajput
 * @param Z lift
 * @param L roll
 * @param M pitch
 * @param N yaw
 */
void apply_equations(int32_t Z, int32_t L, int32_t M, int32_t N) {
#define B 15
#define D 40
#define BD4 4
    ae[0] = (int16_t) square_root((B * N - 2 * D * M + D * Z) / BD4) * 10;
    ae[1] = (int16_t) square_root((2 * D * L - B * N + D * Z) / BD4) * 10;
    ae[2] = (int16_t) square_root((B * N + 2 * D * M + D * Z) / BD4) * 10;
    ae[3] = (int16_t) square_root((-B * N - 2 * D * L + D * Z) / BD4) * 10;
    //do not let the motors stop
    int32_t min_lift = square_root((D * Z) / BD4) * 10;
    int32_t min = min_lift < 180 ? min_lift : 180;
    if (ae[0] < min) {
        ae[0] = min;
    }
    if (ae[1] < min) {
        ae[1] = min;
    }
    if (ae[2] < min) {
        ae[2] = min;
    }
    if (ae[3] < min) {
        ae[3] = min;
    }
}

int a = 0;

/**
 * the control loop
 * by Beni Kovács
 */
void run_filters_and_control() {
    if (mode == MODE_MANUAL) {
        //manual mode
        apply_equations(((int32_t) lift) * 3, -((int32_t) roll) * 3 / 5, -((int32_t) pitch) * 3 / 5,
                        ((int32_t) yaw) * 3);
    } else if (mode == MODE_PANIC) {
        //panic mode
        ae[0] = panicValue;
        ae[1] = panicValue;
        ae[2] = panicValue;
        ae[3] = panicValue;
        a++;
        if (a == 3) {
            a = 0;
            if (panicValue > 0) {
                panicValue--;
            }
            if (panicValue < 120) {
                enterSafeMode();
            }
        }
    } else if (mode == MODE_YAWCONTROL || mode == MODE_FULLCONTROL || mode == MODE_HEIGHTCONTROL || mode == MODE_RAW) {
        //calibrate on first entering raw mode
        //use int32_t to avoid overflow during calculations
        int32_t tsp = (int32_t) sp, tsq = (int32_t) sq, tphi = (int32_t) phi, ttheta = (int32_t) theta, tsr = (int32_t) sr;
        //raw / height mode sensor data processing
        if (mode == MODE_RAW || mode == MODE_HEIGHTCONTROL) {
            //tsr = Fsr;

            //run filters
            ttheta = sax;
            tphi = say;
            kalman_filter(0, &tsp, &tphi);
            kalman_filter(1, &tsq, &ttheta);

            //clear the offsets after 2 seconds when entering raw mode
            if (raw_i < 2000) {
                raw_i++;
            } else if (raw_i == 2000) {
                raw_i++;
                sax_raw_ofst = ttheta;
                say_raw_ofst = tphi;
                tsp_raw_ofst = tsp;
                tsq_raw_ofst = tsq;
                tsr_raw_ofst = tsr;
            }
            ttheta -= sax_raw_ofst;
            tphi -= say_raw_ofst;
            tsp -= tsp_raw_ofst;
            tsq -= tsq_raw_ofst;
            tsr -= tsr_raw_ofst;
        }
        //outputs to pass to apply equations
        int32_t liftEpsilon = 0, rollEpsilon = 0, pitchEpsilon = 0, yawEpsilon = 0;
        //if list is zero, the controllers should not do anything
        if (lift != 0) {
            liftEpsilon = lift * 5;
            rollEpsilon = roll;
            pitchEpsilon = pitch;
            int32_t yawError = yaw - tsr / 10;
            yawEpsilon = yawError * controllerYawP * 3;
            if (mode == MODE_FULLCONTROL || mode == MODE_RAW || mode == MODE_HEIGHTCONTROL) {
                {
                    // roll
                    int32_t rollError = -(((int32_t) roll - (int32_t) tphi / 100) * 150 / 100);
                    rollEpsilon = (int32_t) controllerRollPitchP1 * rollError +
                                  (int32_t) controllerRollPitchP2 * (int32_t) tsp / 100;
                }
                //separate blocks so compiler can optimize memory usage
                {
                    // pitch
                    int32_t pitchError = -(((int32_t) pitch - (int32_t) ttheta / 100) * 150 / 100);
                    pitchEpsilon = (int32_t) controllerRollPitchP1 * pitchError -
                                   (int32_t) controllerRollPitchP2 * (int32_t) tsq / 100;
                }
            }
            //height controller can increase and decrease lift to a given limit
            if (mode == MODE_HEIGHTCONTROL) {
                int32_t delta = (filtered_height - height_to_keep);
                if (delta < -liftEpsilon / 3) {
                    delta = -liftEpsilon / 3;
                } else if (delta > liftEpsilon / 3) {
                    delta = liftEpsilon / 3;
                }
                liftEpsilon += delta;
            }
            pitchEpsilon /= 5;
            rollEpsilon /= 5;
        }
        apply_equations(liftEpsilon, rollEpsilon, pitchEpsilon, yawEpsilon);
    } else {
        //safety first
        ae[0] = 0;
        ae[1] = 0;
        ae[2] = 0;
        ae[3] = 0;
    }
    update_motors();
}

