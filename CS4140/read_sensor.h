#ifndef READ_SENSOR_H
#define READ_SENSOR_H


void read_sensors( void );
void init_raw_timer( void );
void start_raw_timer( void );
void stop_raw_timer( void );

int should_read_sensors;

#endif //READ_SENSOR_H