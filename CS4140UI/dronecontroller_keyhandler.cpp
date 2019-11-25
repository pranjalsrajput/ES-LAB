#include "dronecontroller.h"
#include <QKeyEvent>
#include <QDebug>
#include "com_cpp.h"
#include "ui_dronecontroller.h"

// security: do not let mode changes when something is turned on, show dialog instead
// by Beni Kovács
bool DroneController::checkChangeMode(){

    if(newState.liftKeyboard == 0 && newState.rollKeyboard==0 && newState.yawKeyboard == 0 && newState.pitchKeyboard == 0 &&
            state.liftKeyboard == 0 && state.rollKeyboard==0 && state.yawKeyboard == 0 && state.pitchKeyboard == 0 &&
            joyStickAxis[3] == 100 && joyStickAxis[1] == 0 && joyStickAxis[2] == 0 && joyStickAxis[1] == 0) {
        return true;
    }
    else{
        QMessageBox a;
        a.setText("Press ESC to set all values to 0 before changing mode.");
        a.exec();

    }
}

// by Beni Kovács
// handle key press on main form
void DroneController::keyPressEvent(QKeyEvent* event){
    char buffer[100];
    int outLen = -1;

    char data[10];

    switch (event->key()) {
    case 16777216: //ESC
    case 49: //MODE 1
        newState.liftKeyboard=0;
        newState.rollKeyboard=0;
        newState.pitchKeyboard=0;
        newState.yawKeyboard=0;
        newState.controllerPYawKeyboard=0;
        newState.controllerP1RollPitchKeyboard=0;
        newState.controllerP2RollPitchKeyboard=0;
        newState.mode = 'p';
        break;
    case 48: //MODE 0
        if(checkChangeMode()){
            newState.mode = 's';
        }
        break;
    case 50: //MODE 2
        if(checkChangeMode()){
            newState.mode = 'm';
        }
        break;
    case 51: //MODE 3
        if(checkChangeMode()){
            newState.mode = 'c';
        }
        break;
    case 52: //MODE 4
        if(checkChangeMode()){
            newState.mode = 'y';
            newState.controllerPYawKeyboard = 2;
        }
        break;
    case 53: //MODE 5
        if(checkChangeMode()){
            newState.mode = 'f';
            newState.controllerPYawKeyboard = 1;
            newState.controllerP1RollPitchKeyboard = 5;
            newState.controllerP2RollPitchKeyboard = 20;
        }
        break;
    case 54: //MODE 6
        if(checkChangeMode()){
            newState.mode = 'r';
            newState.controllerPYawKeyboard = 1;
            newState.controllerP1RollPitchKeyboard = 7;
            newState.controllerP2RollPitchKeyboard = 40;
        }
        break;
    case 55: //MODE 7
        if(state.mode == 'h' && newState.mode == 'h'){
            newState.mode = 'r';
        }
        if(newState.mode == 'r' && state.mode == 'r'){
            newState.mode = 'h';
        }
        break;
    case 56: //MODE 8
        newState.wireless = !newState.wireless;
        break;
    case 57: //MODE 9
        constructMessage('t', 0, buffer, &outLen);
        break;
    case 65: //LIFT UP
        if(newState.liftKeyboard < 100){
            newState.liftKeyboard++;
        }
        if(newState.mode=='h'){
//            newState.mode='r';
        }
        break;
    case 90: //LIFT DOWN
        if(newState.liftKeyboard > 0){
            newState.liftKeyboard--;
        }
        if(newState.mode=='h'){
        //    newState.mode='r';
        }
        break;
    case 16777234: //ROLL UP
        if(newState.rollKeyboard < 100){
            newState.rollKeyboard++;
        }
        break;
    case 16777236: //ROLL DOWN
        if(newState.rollKeyboard > -100){
            newState.rollKeyboard--;
        }
        break;
    case 16777235: //PITCH DOWN
        if(newState.pitchKeyboard < 100){
            newState.pitchKeyboard++;
        }
        break;
    case 16777237: //PITCH UP
        if(newState.pitchKeyboard > -100){
            newState.pitchKeyboard--;
        }
        break;
    case 81: //YAW UP
        if(newState.yawKeyboard < 100){
            newState.yawKeyboard++;
        }
        break;
    case 87: //YAW DOWN
        if(newState.yawKeyboard > -100){
            newState.yawKeyboard--;
        }
        break;
    case 85: //YAW CONTROL P UP
        if(newState.controllerPYawKeyboard < 100){
            newState.controllerPYawKeyboard++;
        }
        break;
    case 74: //YAW CONTROL P DOWN
        if(newState.controllerPYawKeyboard >0){
        newState.controllerPYawKeyboard--;
        }
        break;
    case 73: //ROLL/PITCH CONTROL P1 UP
        if(newState.controllerP1RollPitchKeyboard < 100){
            newState.controllerP1RollPitchKeyboard++;
        }
        break;
    case 75: //ROLL/PITCH CONTROL P1 DOWN
        if(newState.controllerP1RollPitchKeyboard >0){
            newState.controllerP1RollPitchKeyboard--;
        }
        break;
    case 79: //ROLL/PITCH CONTROL P2 UP
        if(newState.controllerP2RollPitchKeyboard < 100){
            newState.controllerP2RollPitchKeyboard++;
        }
        break;
    case 76: //ROLL/PITCH CONTROL P2 DOWN
        if(newState.controllerP2RollPitchKeyboard >0){
            newState.controllerP2RollPitchKeyboard--;
        }
        break;
    default:
        break;
    }
    if(outLen > -1){
        writeBufferToSerialPort(buffer, outLen);
    }
}
