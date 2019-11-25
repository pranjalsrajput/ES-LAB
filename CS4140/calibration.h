//
// Author:    	Pranjal Singh Rajput
// Date:    	07/05/2019
//

#ifndef IN4073_CALIBRATION_H
#define IN4073_CALIBRATION_H

#include <inttypes.h>

/* Calibrations Variable*/
#define CALIB_ROUNDS 255

void startCalibration();
void endCalibration();
void loopCalibration();
int calib_i;
void zero_ofsts();

int16_t phi_ofst;
int16_t theta_ofst;
int16_t psi_ofst;

int16_t sp_ofst, sq_ofst, sr_ofst;

int16_t sax_ofst, say_ofst, saz_ofst;

int32_t sax_raw_ofst, say_raw_ofst, tsp_raw_ofst, tsq_raw_ofst, tsr_raw_ofst;


#endif //IN4073_CALIBRATION_H
