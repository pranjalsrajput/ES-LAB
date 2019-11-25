
#include "dronecontroller.h"
#include "ui_dronecontroller.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QSerialPortInfo>
#include<time.h>

#include "joy_cpp.h"
#include "joystickthread.h"

JoyStickThread* jst;

// by Beni Kovács
void DroneController::initJoyStick()
{
    connect(this, SIGNAL(joyStickEvent(int,int,int,int)), this, SLOT(processJoyStickEvent(int,int,int,int)));
    jst = new JoyStickThread();
    jst->start();
    for(int i=0; i<128; i++){
        joyStickAxis[i] = 0;
    }
    joyStickAxis[3] = 100;
#ifndef DRONE_DEBUG
    processJoyStickEvent(2, 0, 3, -100); // sets the lift axis to non-zero so it must moved to zero before takeoff (security)
#endif
}

// by Beni Kovács
void DroneController::deinitJoyStick(){
    jst->quit();
}
