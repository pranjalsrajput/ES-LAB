#include "dronecontroller.h"
#include "ui_dronecontroller.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QSerialPortInfo>
#include<time.h>

#include "com_cpp.h"


// by Beni Kovács
// emit Qt event to sync call to main thread
void DroneController::syncJoyStickEvent(int type, int time, int number, int value){
    emit joyStickEvent(type, time, number, value);
}


// by Beni Kovács
// handle joystick event
void DroneController::processJoyStickEvent(int type, int time, int number, int value){
    char buffer[100];
    int outLen = -1;
    char data[10];
    if(type == 1){
        if(number == 0 && value == 1){
            newState.mode='p';
        }
    }
    if(type == 2){
        joyStickAxis[number] = value / 327;
        if(number==3 && newState.mode=='h'){
            newState.mode='r';
        }
    }
    if(outLen > -1){
        writeBufferToSerialPort(buffer, outLen);
    }

    for(int i=0; i<4; i++){
        //qDebug()<<i<<joyStickAxis[i];
    }
}
