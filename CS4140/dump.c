#include "in4073.h"
#include "comm.h"
#include "control_params.h"
#include "comm_mcu.h"
#include "read_sensor.h"
#include "filter.h"
#include "dump.h"



/**
 * Adds a 2-byte value to successive positions in a byte-array. This function
 * is used to save values in order to save space. To illustrate, if the value
 * that we want to save is 2000 we would need 4 bytes if this number is
 * directly converted to a string.
 *
 * by Stefanos Koffas
 *
 * @param data the byte-array that is used
 * @param value the value that is saved to the byte array
 */
void add_2b_to_int_array(uint8_t *data, int16_t value) {

    data[0] = (uint8_t)((value & 0xff00) >> 8);
    data[1] = (uint8_t)(value);
}

/**
 * Adds a 4-byte value to successive positions in a byte-array. This function
 * is used to save values in order to save space. To illustrate, if the value
 * that we want to save is 2000 we would need 4 bytes if this number is
 * directly converted to a string.
 *
 * by Stefanos Koffas
 *
 * @param data the byte-array that is used
 * @param value the value that is saved to the byte array
 */
void add_4b_to_int_array(uint8_t *data, int32_t value) {

    data[0] = (uint8_t)((value & 0xff000000) >> 24);
    data[1] = (uint8_t)((value & 0xff0000) >> 16);
    data[2] = (uint8_t)((value & 0xff00) >> 8);
    data[3] = (uint8_t)(value);

}

/**
 * Saves to a buffer all the necessary data that are going to be written in the
 * log file that is going to be saved in the flash memory.
 *
 * by Stefanos Koffas
 *
 * @param data the array that every line of this log file is saved
 */

int32_t time = 0;
void prepare_flash_data(uint8_t *data) {

    // Save all the 2-byte values to the flash buffer
    add_2b_to_int_array(&data[0], phi);
    add_2b_to_int_array(&data[2], theta);
    add_2b_to_int_array(&data[4], psi);
    add_2b_to_int_array(&data[6], sp);
    add_2b_to_int_array(&data[8], sq);
    add_2b_to_int_array(&data[10], sr);
    add_2b_to_int_array(&data[12], sax);
    add_2b_to_int_array(&data[14], say);
    add_2b_to_int_array(&data[16], saz);
    add_2b_to_int_array(&data[18], bat_volt);
    add_2b_to_int_array(&data[20], motor[0]);
    add_2b_to_int_array(&data[22], motor[1]);
    add_2b_to_int_array(&data[24], motor[2]);
    add_2b_to_int_array(&data[26], motor[3]);

    // Save all the 4-byte values to flash buffer
    add_4b_to_int_array(&data[28], pressure);
    add_4b_to_int_array(&data[32], temperature);

    add_4b_to_int_array(&data[36], time);
    time++;
}


/**
 * Ensures that the log line saved in data is written to the flash memory.
 *
 * by Stefanos Koffas
 *
 * @param current_position the starting position of the data to be written to
 *                         the flash memory.
 * @param data the buffer that contains the data to be written to the flash
 *             memory
 */
int ensure_bytes_written_to_flash(uint32_t current_position, uint8_t *data) {

    int count = 0;

    while (true) {
        count++;
        if (!flash_write_bytes(current_position, data, LOGGING_LINE_SIZE) && (count < 6)) continue;
        else break;
    }
    if (count >= 6) {
        printf("Flash memory write error.\n");
        flash_write_error = 1;
        return 0;
    }
    return 1;
}