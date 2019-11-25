//
// Created by beni on 2019.05.31..
//
#include "in4073.h"
#include "kalman.h"


#define MAX_FILTERS 2

int32_t _p_b[MAX_FILTERS], _p[MAX_FILTERS], _phi[MAX_FILTERS];

#define C1 100
#define C2 1000*1000

/**
 * initializes kalman filter
 * by Beni Kovács
 */
void init_kalman_filter() {
    P2PHINUM = 1;
    P2PHIDEN = 300;
    for (uint8_t i = 0; i < MAX_FILTERS; i++) {
        _p_b[i] = 0;
        _p[i] = 0;
        _phi[i] = 0;
    }
}

/**
 * appends a new observation to the kalman filter
 * by Beni Kovács
 * @param w the index of the filter to run (to support multiple axis)
 * @param p_sp input & output: velocity
 * @param p_sphi input & output: angle
 */
int32_t int_p = 0;

void kalman_filter(uint8_t w, int32_t *p_sp, int32_t *p_sphi) {
    int32_t _sp = (*p_sp) * (w ? 1 : -1) * 10;
    int32_t _sphi = (*p_sphi) * 1000;
    _p[w] = _sp - _p_b[w];
    _phi[w] = _phi[w] + _p[w] * (P2PHINUM / P2PHIDEN);
    _phi[w] = _phi[w] - (_phi[w] - _sphi) / (C1);
    _p_b[w] = _p_b[w] + (_phi[w] - _sphi) / (C2);
#ifdef DEBUG_KALMAN
    if (w == 0) {
        kalman_1 = _p[w] / 1000;
        kalman_2 = _sphi / 1000;
        kalman_3 = _phi[w] / 1000;
        kalman_4 = _p_b[w];
    }
#endif
    *p_sp = _p[w] * (w ? 1 : -1) / 10;
    *p_sphi = _phi[w] / 1000 * 2 / 3;
}