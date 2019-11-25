//
// Created by beni on 2019.04.30..
//

#ifndef IN4073_CONTROL_PARAMS_H
#define IN4073_CONTROL_PARAMS_H

#include <inttypes.h>
#include "calibration.h"

int8_t lift;
int8_t roll;
int8_t pitch;
int8_t yaw;

int8_t controllerYawP;
int8_t controllerRollPitchP1;
int8_t controllerRollPitchP2;

#define MODE_SAFE 0
#define MODE_PANIC 1
#define MODE_MANUAL 2
//added new mode values
#define MODE_CALIBRATION 3
#define MODE_YAWCONTROL 4
#define MODE_FULLCONTROL 5
#define MODE_HEIGHTCONTROL 7
#define MODE_RAW 8

#define CPANIC 250 // panics mode constant speed

int mode;

int enterPanicTime;
int panicValue;
int lastValidMessageTime;
int stateLedToggleTime;

int lastPhi;
int lastTheta;



int32_t height_to_keep;
//int counter_calib;

#define MODE_WIRED 0
#define MODE_WIRELESS 1

uint8_t comm_mode;

#endif //IN4073_CONTROL_PARAMS_H
