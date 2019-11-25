/*------------------------------------------------------------------
 *  in4073.h -- defines, globals, function prototypes
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#ifndef IN4073_H__
#define IN4073_H__

//#define DEBUG_KALMAN

#include <inttypes.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "ml.h"
#include "app_util_platform.h"
#include <math.h>

#define RED    22
#define YELLOW    24
#define GREEN    28
#define BLUE    30
#define INT_PIN    5

bool demo_done;

// Control
int16_t motor[4], ae[4];

void run_filters_and_control();

// Timers
#define TIMER_PERIOD  50 //50ms=20Hz (MAX 23bit, 4.6h)

void timers_init(void);

uint32_t get_time_us(void);

bool check_timer_flag(void);

void clear_timer_flag(void);

// GPIO
void gpio_init(void);

// Queue
#define QUEUE_SIZE 256
typedef struct {
    uint8_t Data[QUEUE_SIZE];
    uint16_t first, last;
    uint16_t count;
} queue;

void init_queue(queue *q);

void enqueue(queue *q, char x);

char dequeue(queue *q);

// UART
#define RX_PIN_NUMBER  16
#define TX_PIN_NUMBER  14
queue rx_queue;
queue tx_queue;
uint32_t last_correct_checksum_time;

void uart_init(void);

void uart_put(uint8_t);

// TWI
#define TWI_SCL  4
#define TWI_SDA  2

void twi_init(void);

bool i2c_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t const *data);

bool i2c_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t *data);

// MPU wrapper
int16_t phi, theta, psi;
int16_t sp, sq, sr, raw_sr;
int16_t sax, say, saz;
uint8_t sensor_fifo_count;

// FILTER variables

int32_t Fphi, Ftheta, Fpsi;
int32_t Fsp, Fsq, Fsr;
int32_t Fsax, Fsay, Fsaz;

int32_t kalman_1, kalman_2, kalman_3, kalman_4;

int16_t raw_i;

void imu_init(bool dmp,
              uint16_t interrupt_frequency); // if dmp is true, the interrupt frequency is 100Hz - otherwise 32Hz-8kHz
void get_dmp_data(void);

void get_raw_sensor_data(void);

bool check_sensor_int_flag(void);

// Barometer
int32_t pressure, pressure_on_boot, filtered_height, rel_pressure;
int32_t temperature;

void read_baro(void);

void baro_init(void);

// ADC
uint16_t bat_volt;

void adc_init(void);

void adc_request_sample(void);

// Flash
#define LOGGING_LINE_SIZE 40

uint32_t bytes_written_to_flash;
uint32_t bytes_read_from_flash;

uint8_t flash_full;
uint8_t flash_read_error;
uint8_t flash_write_error;
uint8_t flash_start_logging;

bool spi_flash_init(void);

bool flash_chip_erase(void);

bool flash_write_byte(uint32_t address, uint8_t data);

bool flash_write_bytes(uint32_t address, uint8_t *data, uint32_t count);

bool flash_read_byte(uint32_t address, uint8_t *buffer);

bool flash_read_bytes(uint32_t address, uint8_t *buffer, uint32_t count);

// BLE
queue ble_rx_queue;
queue ble_tx_queue;
volatile bool radio_active;

void ble_init(void);

void ble_send(void);

#endif // IN4073_H__
