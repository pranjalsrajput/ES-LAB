#include "joystickthread.h"
#include "joy_cpp.h"

// by Beni Kovács
// background thread to handle joystick events

JoyStickThread::JoyStickThread()
{

}

void JoyStickThread::run(){
    initJoyStickBackend();
}
