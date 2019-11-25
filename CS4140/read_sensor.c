/*
	
	Author: Miguel Perez Avila
	Date: 7/5/2019
	Description: 
	
*/

#include "in4073.h"
#include "read_sensor.h"
#include "control_params.h"
#include "comm.h"
#include "comm_mcu.h"
#include "read_sensor.h"
#include "control_params.h"
#include "filter.h"
#include "app_timer.h"
#include "kalman.h"

#define APP_TIMER_PRESCALER 0


/* Private Functions */
static void phi_calibrated(void);

static void theta_calibrated(void);

static void psi_calibrated(void);

static void sr_calibrated(void);
/**
 * phi_calibrated substract the calibration offset of phi to the last measures and save it in the same address.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
static void phi_calibrated(void) {
    phi = phi - phi_ofst;
}

/**
 * theta_calibrated substract the calibration offset of theta to the last measures and save it in the same address.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
static void theta_calibrated(void) {

    theta = theta - theta_ofst;
}

/**
 * psi_calibrated substract the calibration offset of psi to the last measures and save it in the same address.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
static void psi_calibrated(void) {

    psi = psi - psi_ofst;
}
/**
 * sr_calibrated substract the calibration offset of sr to the last measures and save it in the same address.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
static void sr_calibrated(void) {

    sr = sr - sr_ofst;
}
/**
 * by Pranjal Singh Rajput
 */
void calibrate_height() {

    if (pressure_on_boot == -1) {
        pressure_on_boot = 101500;
        filtered_height = (pressure - pressure_on_boot) * 100;
    }
    rel_pressure = (pressure - pressure_on_boot) * 100;

    filtered_height = 0.95 * filtered_height + 0.05 * rel_pressure;
}
/**
 * read_sensor read all the sensor in the system. Differs in the readings if the mode is different than RAW and HEIGHCONTROL modes or not.
 * If it is not in that modes, it reads for the MPU6000 in dmp mode and calibrate after. If it is in this modes it reads the raw values from the MPU6000 chip
 * , calibrate the values and apply a 50Hz low pass filter.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
void sensor_calibrated(void) {

    phi_calibrated();
    theta_calibrated();
    psi_calibrated();
    sr_calibrated();

    calibrate_height();


}

void raw_sensor_calibrated(void) {
    calibrate_height();
}

uint32_t time_tmp = 0;
uint8_t cntttt = 0;

/**
 * read_sensor read all the sensor in the system. Differs in the readings if the mode is different than RAW and HEIGHCONTROL modes or not.
 * If it is not in that modes, it reads for the MPU6000 in dmp mode and calibrate after. If it is in this modes it reads the raw values from the MPU6000 chip
 * , calibrate the values and apply a 50Hz low pass filter.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
void read_sensors(void) {
    adc_request_sample();
    if (mode != MODE_RAW && mode != MODE_HEIGHTCONTROL) {
        get_dmp_data();
        sensor_calibrated();
        read_baro();
    } else {
        get_raw_sensor_data();
        raw_sensor_calibrated();
        Fsr = FILTER_50HzCutRun(2, sr);
        cntttt++;
        if (cntttt > 10) {
            read_baro();
            cntttt = 0;
        }
    }
}

/**
 * handler for raw timer
 * finally not used, because we were not able to read sensor from this context
 * by Miguel Perez Avila
 * @param p_context
 */
void raw_timer_handler(void *p_context) {
    should_read_sensors = 1;
}

#define RAW_SENSOR_TIMER_PERIOD  APP_TIMER_TICKS(10, APP_TIMER_PRESCALER) // first in milisec
APP_TIMER_DEF(raw_sensor_timer);
/**
 * initializes timer to read raw sensor data with interrupts
 * by Miguel Perez Avila
 */
void init_raw_timer(void) {
    should_read_sensors = 0;
    app_timer_create(&raw_sensor_timer, APP_TIMER_MODE_REPEATED, raw_timer_handler);
}

/**
 * starts timer to read raw sensor data with interrupts
 * by Miguel Perez Avila
 */
void start_raw_timer(void) {
    app_timer_start(raw_sensor_timer, RAW_SENSOR_TIMER_PERIOD, NULL);
}

/**
 * stops timer to read raw sensor data with interrupts
 * by Miguel Perez Avila
 */
void stop_raw_timer(void) {
    app_timer_stop(raw_sensor_timer);
}
