#include "cinterface.h"
#include "dronecontroller.h"


// by Beni Kovács
/**
 * interface for C code for C++ part
 */
void messageReceived(char* ptr, int length){
    DroneController::getInstance()->processReceivedMessage(ptr, length);
}

// by Beni Kovács
/**
 * interface for C code for C++ part
 */
void joyStickEventReceived(int type, int time, int number, int value){
    DroneController::getInstance()->syncJoyStickEvent(type, time, number, value);
}
