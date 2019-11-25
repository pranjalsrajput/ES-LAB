#include "calibration.h"
#include "control_params.h"
#include "in4073.h"
/*
 * calibration: Calibrating sensor values
 * Parameters: 	int selectMode indicates in which mode calibrating
 * Return:   	void, but sets global phi_ofst, theta_ofst, psi_total
 * Author:    	Pranjal Singh Rajput
 * Date:    	07/05/2019
 */

/**
 * by Pranjal Singh Rajput
 */
void zero_ofsts() {
    phi_ofst = 0;
    theta_ofst = 0;
    psi_ofst = 0;
    sp_ofst = 0;
    sq_ofst = 0;
    sr_ofst = 0;
    sax_ofst = 0;
    say_ofst = 0;
    saz_ofst = 0;
    height_to_keep = 0;
}


int32_t phi_total, theta_total, psi_total, sp_total, sq_total, sr_total, sax_total, say_total, saz_total;

/**
 * initilaizes calibration
 * by Pranjal Singh Rajput
 */
void startCalibration() {
    zero_ofsts();

    phi_total = 0;
    theta_total = 0;
    psi_total = 0;
    sp_total = 0;
    sq_total = 0;
    sr_total = 0;
    sax_total = 0;
    say_total = 0;
    saz_total = 0;

    calib_i = 0;

}

/**
 * the calibration loop, should be called after new sensor reading periodically
 * by Pranjal Singh Rajput
 */
void loopCalibration() {
    phi_total += phi;
    theta_total += theta;
    psi_total += psi;

    sp_total += sp;
    sq_total += sq;
    sr_total += sr;

    sax_total += sax;
    say_total += say;
    saz_total += saz;
}

/**
 * ends calibration and sets offsets based on averages
 * by Pranjal Singh Rajput
 */
void endCalibration() {
    phi_ofst = phi_total / CALIB_ROUNDS;
    theta_ofst = theta_total / CALIB_ROUNDS;
    psi_ofst = psi_total / CALIB_ROUNDS;

    sp_ofst = sp_total / CALIB_ROUNDS;
    sq_ofst = sq_total / CALIB_ROUNDS;
    sr_ofst = sr_total / CALIB_ROUNDS;

    sax_ofst = sax_total / CALIB_ROUNDS;
    say_ofst = say_total / CALIB_ROUNDS;
    saz_ofst = saz_total / CALIB_ROUNDS;

    height_to_keep = filtered_height; // averaged for longer interval, better for baro

    //enterSafeMode();
}
