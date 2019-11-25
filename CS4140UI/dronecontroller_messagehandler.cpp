#include "dronecontroller.h"
#include "ui_dronecontroller.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QSerialPortInfo>
#include <QFile>
#include<time.h>
#include "chart.h"

#include "com_cpp.h"

QString output="";

// by Beni Kovács
// show new state on ui
void DroneController::processStateChangeMessage(char* ptr, int length){
    qDebug()<<"Received state change message"<<QString::fromLatin1(QByteArray(ptr, length));
    switch (ptr[0]) {
    case 'c':
        output = "Calibration";
        break;
    case 'p':
        output = "Panic";
        newState.mode = 'p';
        playSound("error");
        break;
    case 'm':
        output = "Manual";
        playSound("manual");
        break;
    case 'y':
        output = "Yaw";
        playSound("yaw_control");
        break;
    case 'f':
        output = "Full";
        playSound("full_control");
        break;
    case 'h':
        output = "Height";
        break;
    case 'r':
        output = "Raw";
        playSound("raw_control");
        break;
    case 's':
        output = "Safe";
        break;
    case 'w':
        state.wireless = !state.wireless;
        break;
    default:
        break;
    }
    if(ptr[0]!='w'){
        state.mode = ptr[0];
    }
    if(state.wireless){
        output = "w"+output;
    }
    ui->lblMode->setText(output);
}

int battery = 0;
int32_t P2PHINUM = 0, P2PHIDEN = 0;

// by Beni Kovács
// show battery V on ui
void DroneController::processReceivedStateRequestMessage(char* ptr, int length){
    battery = ((uint8_t)ptr[0])*20;
    QString number = QString::number(battery);

    ui->lblBattery->setText(QString::number(battery)+", "+QString::number(P2PHINUM)+";"+QString::number(P2PHIDEN));
}


// by Beni Kovács
// show p2phi online tuning values on ui
void DroneController::processReceivedRawConstantMessage(char *ptr, int length) {

    P2PHINUM = ((ptr[0] << 8) & 0xFF00) + ((uint8_t) ptr[1]);
    P2PHIDEN = ((ptr[2] << 8) & 0xFF00) + ((uint8_t) ptr[3]);

    ui->lblBattery->setText(QString::number(battery)+", "+QString::number(P2PHINUM)+";"+QString::number(P2PHIDEN));
}

// @author Stefanos Koffas
// set control values returned by drone on ui
void DroneController::processReceivedControlMessage(char* ptr, int length){
    ui->valLift->setValue(ptr[0]);
    ui->valRoll->setValue(ptr[1]);
    ui->valPitch->setValue(ptr[2]);
    ui->valYaw->setValue(ptr[3]);
    ui->valPYaw->setValue(ptr[4]);
    ui->valP1RP->setValue(ptr[5]);
    ui->valP2RP->setValue(ptr[6]);
}
/**
 * processChartMessage It reads from a buffer the data and add it to the real time chart for visualization.
 * @author Miguel Pérez Ávila.
 * @param  char* ptr. pointer to the data.
 * @param  int length. Length of the data to be process.
 * @return void.
 * */
void DroneController::processChartMessage(char *ptr, int length){
    QList<int> data;
    int16_t tmp = 0;
    //for(int i=0; i < 3; i++){
    //    data << ptr[i];
    //}
    tmp =((ptr[0]<<8)&0xFF00) +((uint8_t)ptr[1] );
    data << tmp;
    tmp = 0;

    tmp =((ptr[2]<<8)&0xFF00) +((uint8_t)ptr[3] );
    data << tmp;
    tmp = 0;

    tmp =((ptr[4]<<8)&0xFF00) +((uint8_t)ptr[5] );
    data << tmp;
    tmp = 0;

    tmp =((ptr[6]<<8)&0xFF00) +((uint8_t)ptr[7] );
    data << tmp;
    tmp = 0;

    tmp =((ptr[8]<<8)&0xFF00) +((uint8_t)ptr[9] );
    data << tmp;
    tmp = 0;

    tmp =((ptr[10]<<8)&0xFF00) +((uint8_t)ptr[11] );
    data << tmp;
    tmp = 0;

    qDebug()<<data<<"processCharMessage";
    chartView->addChartData(data);
}

// @author Stefanos Koffas
// process logging message
void DroneController::processLoggingControlMessage(char *ptr, int length){

    switch (ptr[0]) {
    case 's':
        qDebug() << "Logging has started";
        state.logging_state = 's';
        break;
    case 'x':
        qDebug() << "Logging has stopped";
        state.logging_state = 'x';
        break;
    default:
        break;
    }
}

// @author Stefanos Koffas
// show bytes number written to log
void DroneController::processLoggingSizeMessage(char *ptr, int length) {

    logging_length = 0;

    logging_length = ptr[0];
    logging_length += ((ptr[1] << 8) & 0xFF00);
    logging_length += ((ptr[2] << 16) & 0xFF0000);
    logging_length += ((ptr[3] << 24) & 0xFF000000);

    qDebug() << logging_length << "bytes written so far";

    // Request a log dump in the next interrupt
    newState.logging_dump_request = logging_length / LOGGING_LINE_SIZE;
}

// @author Stefanos Koffas
// decode bytes to int16
int16_t DroneController::extract_2b_from_buffer(char *ptr) {
    return ((ptr[0]<<8)&0xFF00) | ((uint8_t)ptr[1] );
}

// @author Stefanos Koffas
// decode bytes to int32
int32_t DroneController::extract_4b_from_buffer(char *ptr) {
    return  ((ptr[0]<<24)&0xFF000000) | ((ptr[1]<<16)&0xFF0000) | ((ptr[2]<<8)&0xFF00) | ((uint8_t)ptr[3] );
}


/**
 * Write an incoming line of the log file to the specified log file given in
 * DroneController::logfile_name.
 * TODO: Find a better way to handle these messages because our framework
 * cannot handle all the packets that come from the microcontroller. Maybe we
 * can use a special loop for saving the entire incoming buffer to a file.
 * @param ptr the payload of a dump chunk
 * @param length the length of this payload
 */
void DroneController::processLoggingDumpMessage(char *ptr, int length) {

    int16_t phi, theta, psi, sp, sq, sr, sax, say, saz, bat_volt, m1, m2, m3, m4;
    int32_t pressure, temperature;
    uint32_t time;

    QFile file(logfile_name);

    // Trying to open in WriteOnl and Text mode
    if(!file.open(QFile::WriteOnly | QFile::Text | QFile::Append)) {
        qDebug() << " Could not open file for writing";
        return;
    }else{
        qDebug() << "file open";
    }
    QTextStream log(&file);
    if(file.size() == 0){
        log << "headers = ['time[ms]', 'phi', 'theta', 'psi', tsp', 'tsq', 'sr', 'sax', 'say', 'saz', 'bat_volt', 'pressure', 'temperature', 'motor1', 'motor2', 'tmotor3', 'motor4']\n";
    }

    phi = extract_2b_from_buffer(&ptr[0]);
    theta = extract_2b_from_buffer(&ptr[2]);
    psi = extract_2b_from_buffer(&ptr[4]);
    sp = extract_2b_from_buffer(&ptr[6]);
    sq = extract_2b_from_buffer(&ptr[8]);
    sr = extract_2b_from_buffer(&ptr[10]);
    sax = extract_2b_from_buffer(&ptr[12]);
    say = extract_2b_from_buffer(&ptr[14]);
    saz = extract_2b_from_buffer(&ptr[16]);
    bat_volt = extract_2b_from_buffer(&ptr[18]);
    m1 = extract_2b_from_buffer(&ptr[20]);
    m2 = extract_2b_from_buffer(&ptr[22]);
    m3 = extract_2b_from_buffer(&ptr[24]);
    m4 = extract_2b_from_buffer(&ptr[26]);

    pressure = extract_4b_from_buffer(&ptr[28]);
    temperature = extract_4b_from_buffer(&ptr[32]);
    time = extract_4b_from_buffer(&ptr[36]);

    log << "[" << time << ", " << phi << ", " << theta
        << ", " << psi << ", " << sp << ", " << sq << ", "
        << sr << ", " << sax << ", " << say << ", " << saz
        << ", " << bat_volt << ", " << pressure << ", "
        << temperature << ", " << m1 <<", "<< m2 << ", "
        << m3 <<", " << m4 << "],\n";

    file.flush();
    file.close();

    bytes_read += LOGGING_LINE_SIZE;

    state.logging_dump_request ++;
}

// by Beni Kovács
// main switch for message handling
void DroneController::processReceivedMessage(char* ptr, int length){
    QString message = "";
    message.append(ptr[0]);
    for(int i=1; i<length; i++){
        message.append(" ");
        message.append(QString::number(ptr[i]));
    }
    qDebug()<<"message split:  "+message;
    switch(ptr[0]) {
    case 's':
        processReceivedStateRequestMessage(ptr+1, length-1);
        break;
    case 'm':
        processReceivedControlMessage(ptr+1, length-1);
        break;
    case 'c':
        processStateChangeMessage(ptr+1, length-1);
        break;
    case 'a':
        processChartMessage(ptr+1, length-1);
        break;
    case 'l':
        processLoggingControlMessage(ptr+1, length-1);
        break;
    case 'b':
        processLoggingSizeMessage(ptr+1, length-1);
        break;
    case 'd':
        processLoggingDumpMessage(ptr+1, length-1);
        break;
    case 'r':
        processReceivedRawConstantMessage(ptr+1, length-1);
        break;
    default:
        break;
    }
}
