#include "dronecontroller.h"
#include "ui_dronecontroller.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QSerialPortInfo>
#include<time.h>

#include "com_cpp.h"

// TODO: This constant is needed because when we dump the log file from the
// microcontroller in lines of LOGGING_LINE_SIZE a lot of bytes are read from
// the serial port. The previous limit of 65 was too low for this purpose
#define INPUT_SIZE 100000

QElapsedTimer tmr;

// @author Stefanos Koffas
// init serial port for data transfer
bool DroneController::initSerialPort(){
    serialPort = new QSerialPort();
    serialPort->setPortName("/dev/ttyUSB0");
    serialPort->setBaudRate(115200);
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readyRead()));

    return serialPort->open(QIODevice::ReadWrite);
}

// @author Stefanos Koffas
// event invoked by Qt API if there is new message
void DroneController::readyRead(){
    QByteArray incoming = serialPort->readAll();
    qDebug() << incoming.data();
    qDebug() << incoming << "Incoming Message\r\n";
    if(incoming.size()<INPUT_SIZE){
        processIncoming(incoming.data(), incoming.size());
    }
}

// @author Stefanos Koffas
// event writes char* buffer to Qt serial port
void DroneController::writeBufferToSerialPort(char* buffer, int outLen){
    serialPort->write(buffer, outLen);
    tmr.start();
    QByteArray out(buffer, outLen);
    //qDebug()<<"written"<<out;
}

// by Beni Kovács
// limit output values
int8_t minMax100(int in){
    if(in < -100){
        in = -100;
    } else if (in > 100){
        in = 100;
    }
    return (int8_t)in;
}

// by Beni Kovács
// send new state to output
void DroneController::timerExpired(){

    char buffer[10];
    int outLen;

    if (newState.wireless != state.wireless){
        qDebug()<<"writing mode change request";
        char w = 'w';
        constructMessage('c', &w, buffer, &outLen);
        writeBufferToSerialPort(buffer, outLen);
    }

    if(state.wireless){
        return;
    }

    if (newState.mode != state.mode){
        qDebug()<<"Requesting mode change from"<<state.mode<<"to"<<newState.mode;
        constructMessage('c', &newState.mode, buffer, &outLen);
        writeBufferToSerialPort(buffer, outLen);
    }
    constructMessage('s', 0, buffer, &outLen);
    writeBufferToSerialPort(buffer, outLen);
#ifdef DRONE_DEBUG

    if(ui->cbRequestChartData->checkState() == Qt::Checked){
        constructMessage('a', 0, buffer, &outLen);
        writeBufferToSerialPort(buffer, outLen);
    }

#endif


    char data[8];
    if (newState.logging_state != state.logging_state) {
        if (newState.logging_state == 's') {
            qDebug() << "Start logging requested";
        } else if (newState.logging_state == 'x') {
            qDebug() << "Stop logging  requested";
        }
        constructMessage('l', &newState.logging_state, buffer, &outLen);
        writeBufferToSerialPort(buffer, outLen);
    }

    if (newState.logging_size_request) {
        newState.logging_size_request = false;
        constructMessage('b', 0, buffer, &outLen);
        writeBufferToSerialPort(buffer, outLen);
    }

    if (newState.logging_dump_request > state.logging_dump_request) {
        qDebug() << "another chunk is requested";
        constructMessage('d', 0, buffer, &outLen);
        writeBufferToSerialPort(buffer, outLen);
    }


#ifdef DRONE_DEBUG

    static int sent=-1;

    if(ui->tmp->value()!=sent){
        char d[4];
        d[2] = (uint8_t)((1 & 0xff00) >> 8);
        d[3] = (uint8_t)(1);

        d[0] = (uint8_t)((ui->tmp->value() & 0xff00) >> 8);
        d[1] = (uint8_t)(ui->tmp->value());


        constructMessage('r', d, buffer, &outLen);
        writeBufferToSerialPort(buffer, outLen);
        sent = ui->tmp->value();
    }
#endif


    int16_t lift = minMax100(newState.liftKeyboard + (-joyStickAxis[3]+100)/2);
    int16_t roll = minMax100(newState.rollKeyboard + joyStickAxis[0]);
    int16_t pitch = minMax100(newState.pitchKeyboard + joyStickAxis[1]);
    int16_t yaw = minMax100(newState.yawKeyboard-joyStickAxis[2]);
    data[0] = lift;
    data[1] = roll;
    data[2] = pitch;
    data[3] = yaw;
    data[4] = newState.controllerPYawKeyboard;
    data[5] = newState.controllerP1RollPitchKeyboard;
    data[6] = newState.controllerP2RollPitchKeyboard;
    constructMessage('m', data, buffer, &outLen);
    writeBufferToSerialPort(buffer, outLen);
}
