//
// Created by beni on 2019.05.31..
//

#ifndef IN4073_KALMAN_H
#define IN4073_KALMAN_H

void init_kalman_filter();
void kalman_filter(uint8_t w, int32_t *p_sp, int32_t *p_sphi);

int32_t P2PHINUM;
int32_t P2PHIDEN;

#endif //IN4073_KALMAN_H
