//
// Created by beni on 2019.05.29..
//

#ifndef IN4073_DUMP_H
#define IN4073_DUMP_H

int ensure_bytes_written_to_flash(uint32_t current_position, uint8_t *data);
void prepare_flash_data(uint8_t *data);
void add_4b_to_int_array(uint8_t *data, int32_t value);
void add_2b_to_int_array(uint8_t *data, int16_t value);
#endif //IN4073_DUMP_H
