//
// Created by beni on 2019.04.30..
//

#include "control_params.h"
#include "in4073.h"
#include "comm.h"
#include "comm_mcu.h"
#include "read_sensor.h"
#include "kalman.h"

char buffer[40];
int len = -1;

/***
 * prints message to the otput
 * by Beni Kovács
 * @param ptr pointer to the 1st char of the message
 * @param length explicit length, force write until this number of bytes are written (message may contain \0-s)
 */
void printMessage(char *ptr, int length) {
    // since android app does not support feedback, print always to computer
    for (int i = 0; i < length; i++) {
        printf("%c", ptr[i]);
    }
    printf("\n");
}

/***
 * sends back the actual control values to the computer
 * by Beni Kovács
 */
void sendControlAcknowledgement() {
    char d[8];
    d[0] = lift;
    d[1] = roll;
    d[2] = pitch;
    d[3] = yaw;
    d[4] = controllerYawP;
    d[5] = controllerRollPitchP1;
    d[6] = controllerRollPitchP2;
    constructMessage('m', d, buffer, &len);
    printMessage(buffer, len);
}

/***
 * put int16_t to byte buffer
 * by Miguel Perez Avila
 * @param num the number to send through serial port
 * @param target pointer to buffer poisition
 */
void int_16_to_chars(int16_t num, char *target) {
    target[0] = (uint8_t)((num & 0xff00) >> 8);
    target[1] = (uint8_t)(num);
}

/***
 * this function sets the value of P2PHI to tune online
 * by Beni Kovács
 * @param ptr pointer of the useful parts of the message (start char, message type truncated)
 * @param length length of the useful message part (trailed parts and checksum not included)
 */
void processReceivedRawConstantMessage(char *ptr, int length) {
#ifdef DEBUG_KALMAN

    P2PHINUM = ((ptr[0] << 8) & 0xFF00) + ((uint8_t) ptr[1]);
    P2PHIDEN = ((ptr[2] << 8) & 0xFF00) + ((uint8_t) ptr[3]);
#endif

    char d[4];

    int_16_to_chars(P2PHINUM, d);
    int_16_to_chars(P2PHIDEN, d + 2);

    constructMessage('r', d, buffer, &len);
    printMessage(buffer, len);
}

/***
 * set values from keyboard / joystick / phone
 * by Beni Kovács
 * @param ptr pointer of the useful parts of the message (start char, message type truncated)[0] = (uint8_t)((P2PHINUM & 0xff00) >> 8);
    d[1] = (uint8_t)(P2PHINUM);
 * @param length length of the useful message part (trailed parts and checksum not included)
 */
void processReceivedControlMessage(char *ptr, int length) {
    lift = ptr[0];
    roll = ptr[1];
    pitch = ptr[2];
    yaw = ptr[3];
    controllerYawP = ptr[4];
    controllerRollPitchP1 = ptr[5];
    controllerRollPitchP2 = ptr[6];
    //always acknowledge
    sendControlAcknowledgement();
}

/***
 * enters manual mode
 * by Beni Kovács
 */
void enterManualMode() {
    mode = MODE_MANUAL;
    nrf_gpio_pin_clear(GREEN);
    nrf_gpio_pin_set(RED);
    sendModeAcknowledgement('m');
}

/***
 * enters yaw control mode
 * by Beni Kovács
 */
void enterYawControlMode() {
    mode = MODE_YAWCONTROL;
    nrf_gpio_pin_clear(GREEN);
    nrf_gpio_pin_set(RED);
    sendModeAcknowledgement('y');
}

/***
 * enters calibration mode
 * by Beni Kovács
 */
void enterCalibrationMode() {
    mode = MODE_CALIBRATION;
    nrf_gpio_pin_clear(GREEN);
    nrf_gpio_pin_set(RED);
    sendModeAcknowledgement('c');
    startCalibration();
}

/***
 * enters full control mode
 * by Beni Kovács
 */
void enterFullControlMode() {
    mode = MODE_FULLCONTROL;
    nrf_gpio_pin_clear(GREEN);
    nrf_gpio_pin_set(RED);
    sendModeAcknowledgement('f');
}

/***[0] = (uint8_t)((P2PHINUM & 0xff00) >> 8);
    d[1] = (uint8_t)(P2PHINUM);
 * enters height control mode
 * by Beni Kovács
 */
void enterHeightControlMode() {
    height_to_keep = filtered_height;
    mode = MODE_HEIGHTCONTROL;
    nrf_gpio_pin_clear(GREEN);
    nrf_gpio_pin_set(RED);
    sendModeAcknowledgement('h');
}

/***
 * min function with int16_t
 * by Beni Kovács
 * @param a first number
 * @param b second number
 * @return the smaller number from a and b
 */
int16_t min(int16_t a, int16_t b) {
    return a < b ? a : b;
}

/***
 * enters panic mode
 * by Beni Kovács
 */
void enterPanicMode() {
    mode = MODE_PANIC;
    panicValue = min(motor[0], min(motor[1], min(motor[2], motor[3])));
    enterPanicTime = get_time_us();
    nrf_gpio_pin_set(GREEN);
    nrf_gpio_pin_clear(RED);
    sendModeAcknowledgement('p');
}

/***
 * enters safe mode
 * by Beni Kovács
 */
void enterSafeMode() {
    nrf_gpio_pin_set(GREEN);
    nrf_gpio_pin_set(RED);
    mode = MODE_SAFE;
    enterPanicTime = 0;

    sendModeAcknowledgement('s');
}

/***
 * enters raw mode
 * by Beni Kovács
 */
void enterRawMode(void) {
    mode = MODE_RAW;
    nrf_gpio_pin_clear(GREEN);
    nrf_gpio_pin_set(RED);
    imu_init(false, 1000);
    sendModeAcknowledgement('r');
}

/***
 * sends acknowledgement of mode change
 * by Beni Kovács
 * @param mode
 */
void sendModeAcknowledgement(char mode) {
    constructMessage('c', &mode, buffer, &len);
    printMessage(buffer, len);
}

/***
 * toggles the communication channel for incoming data
 * Pranjal Singh Rajput
 */
void enterWirelessMode() {
    comm_mode = (comm_mode + 1) % 2;
    sendModeAcknowledgement('w');
}

/***
 * sends battery voltage
 * by Beni Kovács
 * @param ptr this function receives no input, so maybe anything
 * @param length 0 for this message type
 */
void processReceivedStateRequestMessage(char *ptr, int length) {
    char data[2];
    data[0] = (uint8_t)(bat_volt / 20);
    constructMessage('s', data, buffer, &len);
    printMessage(buffer, len);
}

/***
 * checks if all rotors are turned off
 * by Pranjal Singh Rajput
 * @return true if not motors are energized
 */
int8_t rotors_off() {
    int8_t ok = 1;
    for (uint8_t i = 0; i < 4; i++) {
        if (ae[i] != 0) {
            ok = 0;
        }
    }
    return ok;
}

/***
 * processes incoming state change request
 * by Beni Kovács
 * @param ptr beginning of the useful message
 * @param length length of the useful message
 */
void processStateChangeMessage(char *ptr, int length) {
    int disable_raw_mode = 0;
    // we use a switch due to safety reasons, so we don't need function pointers
    switch (ptr[0]) {
        case 'c': //calibrate
            if (mode == MODE_SAFE) {
                enterCalibrationMode();
            }
            break;
        case 'p': //panic
            if (mode != MODE_SAFE && mode != MODE_PANIC) {
                disable_raw_mode = mode == MODE_RAW;
                enterPanicMode();
            }
            break;
        case 'm': //manual
            if (mode == MODE_SAFE && rotors_off()) {
                enterManualMode();
            }
            break;
        case 'y': //yaw closed loop
            if (mode == MODE_SAFE && rotors_off()) {
                enterYawControlMode();
            }
            break;
        case 'f': //full closed loop
            if (mode == MODE_SAFE && rotors_off()) {
                enterFullControlMode();
            }
            break;
        case 'h': //full closed loop + height control
            if (mode == MODE_RAW) {
                enterHeightControlMode();
            }
            break;
        case 'r': //raw
            if (mode == MODE_SAFE || mode == MODE_HEIGHTCONTROL) {
                if(mode==MODE_SAFE){
                    raw_i = 0;
                }
                enterRawMode();
            }
            break;
        case 'w':
            enterWirelessMode();
            break;
        case 's': //safe
            if (mode != MODE_PANIC) {
                disable_raw_mode = mode == MODE_RAW;
                enterSafeMode();
            }
            break;
        default:
            break;
    }
    if (disable_raw_mode) {
        imu_init(true, 100);
    }
}

/***
 * writes motor values for debugging purposes. this function is deprecated and was replaced with reportValuesForDebug
 * by Beni Kovács
 */
void writeMotorValues() {
    char data[8];
    for (uint8_t i = 0; i < 4; i++) {
        int32_t k = ae[i] / 10;
        if (k > 127) {
            k = 127;
        } else if (k < -127) {
            k = -127;
        }
        data[i] = k;
    }
    constructMessage('a', data, buffer, &len);
    printMessage(buffer, len);
}

/**
 * reportValueForDebug. This function send back to the desktop different values to debug in real-time.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
void reportValuesForDebug(void) {
    char data[12];

    int_16_to_chars(ae[0], data);
    int_16_to_chars(ae[1], data + 2);
    int_16_to_chars(ae[2], data + 4);
    int_16_to_chars(ae[3], data + 6);

    int_16_to_chars(0, data + 8);
    int_16_to_chars(0, data + 10);

#ifdef DEBUG_KALMAN

    int_16_to_chars(0, data);
    int_16_to_chars(0, data + 2);
    int_16_to_chars(kalman_1, data + 4);
    int_16_to_chars(kalman_2, data + 6);
    int_16_to_chars(kalman_3, data + 8);
    int_16_to_chars(kalman_4, data + 10);

#endif

    int_16_to_chars((int16_t)(sr_ofst), data + 8);
    int_16_to_chars((int16_t)(sr), data + 10);

    constructMessage('a', data, buffer, &len);
    printMessage(buffer, len);
}

/**
 * Send the number to the pc the number of bytes that have been written to the
 * log file
 * by Stefanos Koffas
 */
void send_log_size_to_pc() {

    char data[4];

    data[0] = bytes_written_to_flash & 0xFF;
    data[1] = (bytes_written_to_flash >> 8) & 0xFF;
    data[2] = (bytes_written_to_flash >> 16) & 0xFF;
    data[3] = (bytes_written_to_flash >> 24);

    constructMessage('b', data, buffer, &len);
    printMessage(buffer, len);
}

/**
 * Starts the logging functionality
 * by Stefanos Koffas
 */
void start_logging() {

    char data[1];

    flash_start_logging = 1;
    data[0] = 's';
    constructMessage('l', data, buffer, &len);
    printMessage(buffer, len);
}

/**
 * Stops the logging functionality
 * by Stefanos Koffas
 */
void stop_logging() {

    char data[0];

    flash_start_logging = 0;
    data[0] = 'x';
    constructMessage('l', data, buffer, &len);
    printMessage(buffer, len);
}


/**
 * Enables or disables logging functionality to the flash memory
 * by Stefanos Koffas
 * @param ptr buffer for the message payload
 * @param length length of the payload
 */
void processReceivedLogControlMessage(char *ptr, int length) {

    switch (ptr[0]) {
        case 's': // start logging
            start_logging();
            break;
        case 'x': // stop logging
            stop_logging();
            break;
        default:
            break;
    }
}

void processReceivedTrimRequestMessage(char *ptr, int length) {
}

/**
 * Sends to the PC the number of bytes that have been written to the log file
 * so that the PC side is aware of what to expect when requesting the log dump
 * by Stefanos Koffas
 * @param ptr buffer for the message payload
 * @param length length of the payload
 */
void processReceivedLogSizeMessage(char *ptr, int length) {

    send_log_size_to_pc();
}

/**
 * Ensures that the log line is read from the flash memory. If the specific
 * chunk of data could not be read this function returns an error.
 * by Stefanos Koffas
 * @param address the starting address from which the data is read.
 * @param buffer pointer to uint8_t type array where the data is stored
 * @return
 * @retval 0 if operation is not successful
 * @retval 1 if operation is sucessful
 */
uint8_t ensure_read_bytes(uint32_t address, char *buffer) {

    uint8_t count = 0;

    while (true) {
        // TODO: Make sure that this type cast is ok
        if (!flash_read_bytes(address, (uint8_t *) buffer, LOGGING_LINE_SIZE) && (count < 6)) continue;
        else break;
    }
    if (count >= 6) {
        printf("Flash memory read error.\n");
        flash_read_error = 1;
        bytes_read_from_flash = 0;
        return 0;
    }
    return 1;
}

/**
 * Dumps all the log file in chunks of LOGGING_LINE_SIZE to the communication
 * channel
 * by Stefanos Koffas
 * @param ptr buffer for the message payload
 * @param length length of the payload
 */
void processReceivedLogDumpMessage(char *ptr, int length) {

    char data[LOGGING_LINE_SIZE];
    //uint32_t starting_position, end_position;
    //int i;

    // If previously there was an error in the log dump this should not affect
    // a new request for log dump. So assign this value every time we receive a
    // log dump request.
    flash_read_error = 0;

    if ((bytes_read_from_flash + LOGGING_LINE_SIZE) < bytes_written_to_flash && !flash_read_error) {
        if (ensure_read_bytes(bytes_read_from_flash, data)) {
            bytes_read_from_flash += LOGGING_LINE_SIZE;
            constructMessage('d', data, buffer, &len);
            printMessage(buffer, len);
        }
    } else {
        // start from the beginning in the next iteration
        bytes_read_from_flash = 0;
    }
}

/**
 * this function is called when a valid message is received
 * by Beni Kovács
 * @param ptr pointer to received message: 1st element: action char, all remaining: data
 * @param length length of message, including action char, not including checksum and start bit
 */

void messageReceived(char *ptr, int length) {
    lastValidMessageTime = get_time_us();
    //use switch because it is very robust, so we don't need function pointers, etc..
    switch (ptr[0]) {
        case 's':
            processReceivedStateRequestMessage(ptr + 1, length - 1);
            break;
        case 'm':
            processReceivedControlMessage(ptr + 1, length - 1);
            break;
        case 'c':
            // TODO: copy this template increase pointer to process useful data, decrease remaining length
            processStateChangeMessage(ptr + 1, length - 1);
            break;
        case 'a':
            //writeMotorValues();
            //printf("ReportValuesForDebug\r\n");
            reportValuesForDebug();
            break;
        case 't': //TRIM raw sensor readings
            processReceivedTrimRequestMessage(ptr + 1, length - 1);
            break;
        case 'l':
            processReceivedLogControlMessage(ptr + 1, length - 1);
            break;
        case 'b':
            processReceivedLogSizeMessage(ptr + 1, length - 1);
            break;
        case 'd':
            processReceivedLogDumpMessage(ptr + 1, length - 1);
            break;
        case 'r':
            processReceivedRawConstantMessage(ptr + 1, length - 1);
        default:
            break;
    }
}
