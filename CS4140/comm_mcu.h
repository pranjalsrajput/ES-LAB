//
// Created by beni on 2019.04.30..
//

#include "in4073.h"
#include "comm.h"
#include "control_params.h"

#ifndef IN4073_COMM_MCU_H
#define IN4073_COMM_MCU_H

void messageReceived(char* ptr, int length);
void printMessage(char* ptr, int length);

void enterManualMode();
void enterPanicMode();
void enterSafeMode();
void enterRawMode( void );

void sendModeAcknowledgement(char mode);

#endif //IN4073_COMM_MCU_H
