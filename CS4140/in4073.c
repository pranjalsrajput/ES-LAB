/*------------------------------------------------------------------
 *  in4073.c -- test QR engines and sensors
 *
 *  reads ae[0-3] uart rx queue
 *  (q,w,e,r increment, a,s,d,f decrement)
 *
 *  prints timestamp, ae[0-3], sensors to uart tx queue
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"
#include "comm.h"
#include "control_params.h"
#include "comm_mcu.h"
#include "read_sensor.h"
#include "filter.h"
#include "dump.h"
#include "kalman.h"

/*------------------------------------------------------------------
 * main -- everything you need is here :)
 *------------------------------------------------------------------
 */

int batt_errors = 0;
/***
 * checks if battery has depleted, with some filtering (turns off if it's below limit for 0.4s)
 * by Beni Kovács
 * @return battery state int
 * @retval 0: battery depleted
 * @retval 1: battery is charged
 */
int batt_volt_ok() {
    //return 1;
    if (bat_volt > 1060) {
        batt_errors--;
    } else {
        batt_errors++;
    }
    if (batt_errors < 0) {
        batt_errors = 0;
    }

    //in raw mode it runs 10 times more frequent
    if (batt_errors > ((mode == MODE_RAW || mode == MODE_HEIGHTCONTROL) ? 400 : 40)) {
        return 0;
    } else {
        return 1;
    }

}

/**
 * Write log data
 * by Stefanos Koffas
 */
void prepare_logging(){

    if (flash_start_logging) {
        //printf("str");
        if (!flash_full && !flash_write_error && flash_start_logging) {
            uint8_t data[LOGGING_LINE_SIZE];
            if ((bytes_written_to_flash + LOGGING_LINE_SIZE) > 0x01FFFF) {
                printf("mem_full");
                flash_full = 1;
            } else {
                prepare_flash_data(data);
                if (ensure_bytes_written_to_flash(bytes_written_to_flash, data)) {
                    bytes_written_to_flash += LOGGING_LINE_SIZE;
                }
            }
        } else {
            printf("err");
        }
    }
}

int main(void) {
    uart_init();
    gpio_init();
    timers_init();
    adc_init();
    twi_init();
    imu_init(true, 100);
    baro_init();
    spi_flash_init(); // todo: check always if they return true and send error when not true
    flash_chip_erase();
    ble_init();

    sax_raw_ofst = 0;
    say_raw_ofst = 0;
    tsp_raw_ofst = 0;
    tsq_raw_ofst = 0;
    tsr_raw_ofst = 0;

    init_kalman_filter();

    zero_ofsts();

    FILTER_50HzCutInit();

    //uint32_t counter = 0;
    demo_done = false;

    enterPanicTime = 0;
    stateLedToggleTime = 0;
    lastValidMessageTime = 0;
    mode = MODE_SAFE;
    roll = 0;
    pitch = 0;
    yaw = 0;
    lift = 0;

    sax_ofst = 0;
    say_ofst = 0;
    saz_ofst = 0;

    comm_mode = MODE_WIRED;
    controllerYawP = 0;
    controllerRollPitchP1 = 0;
    controllerRollPitchP2 = 0;
    bytes_written_to_flash = 0;
    bytes_read_from_flash = 0;
    flash_full = 0;
    flash_write_error = 0;
    flash_read_error = 0;
    calib_i = CALIB_ROUNDS + 1;

    pressure_on_boot = -1;

    enterSafeMode();

    while (!demo_done) {
        int time = get_time_us();

        if (check_sensor_int_flag()) {

            if ((mode != MODE_SAFE && mode != MODE_PANIC) &&
                (!batt_volt_ok() || time > lastValidMessageTime + 1000000)) {
                enterPanicMode();
                if(mode == MODE_RAW || mode == MODE_HEIGHTCONTROL){
                    imu_init(true, 100);
                }
            }

            read_sensors();
            run_filters_and_control();

            if (calib_i < CALIB_ROUNDS) {
                loopCalibration();
                calib_i++;
            }
        }

        if (check_timer_flag()) {
            clear_timer_flag();

            // Transform the data
            // Open the file in append mode
            // Write the data (circularly)
            //for (int i = 0; i < 9999999; i++) {

            //}
            prepare_logging();
        }

        if (rx_queue.count) {
            char c = dequeue(&rx_queue);
            processIncoming(&c, 1);
        }

        // in mode wireless read BT to (serial is read always, because of safety)
        if (comm_mode == MODE_WIRELESS && ble_rx_queue.count) {
            char c = dequeue(&ble_rx_queue);
            processIncoming(&c, 1);
        }


        // flash blue
        if (time > stateLedToggleTime + 500000) {
            nrf_gpio_pin_toggle(BLUE);
            stateLedToggleTime = time;
        }

        // finish calibration
        if (calib_i == CALIB_ROUNDS) {
            endCalibration();
            calib_i++;
        }
    }
}
