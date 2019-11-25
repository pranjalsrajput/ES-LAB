#ifndef DRONECONTROLLER_H
#define DRONECONTROLLER_H

#include <QWidget>
#include <QSerialPort>
#include "chart.h"
#include "loggingChart.h"

namespace Ui {
class DroneController;
}

#define DRONE_DEBUG

struct State {
    char mode = '-';
    int8_t controllerPYawKeyboard = 0;
    int8_t controllerP1RollPitchKeyboard = 0;
    int8_t controllerP2RollPitchKeyboard = 0;
    int8_t liftKeyboard = 0;
    int8_t rollKeyboard = 0;
    int8_t pitchKeyboard = 0;
    int8_t yawKeyboard = 0;
    bool wireless = false;
    char logging_state = '-';
    bool logging_size_request = false;
    int logging_dump_request = 0;
};

class DroneController : public QWidget
{
    Q_OBJECT

public:
    explicit DroneController(QWidget *parent = 0);
    ~DroneController();
    void processReceivedMessage(char* ptr, int length);

    void syncJoyStickEvent(int type, int time, int number, int value);

    void playSound(QString name);

signals:
    void joyStickEvent(int type, int time, int number, int value);

private:
    Ui::DroneController *ui;

    Chart* chartView;
    LoggingChart* loggingChartView;


    // TODO: panic button on joystick not working (it was already working)
    // TODO: joystick values are not zero in initial state
    // TODO: change to manual state only iff all control value are 0

    // values sent regardless drone values

    // values send only on change
    State newState;
    State state;

    // Logging file
    QString logfile_name = "../CS4140UI/logs.txt";
    // Bytes written so far
    uint32_t logging_length;
    uint32_t bytes_read = 0;

    bool initSerialPort();
    void initJoyStick();
    void deinitJoyStick();

    static DroneController* instance;

    QSerialPort* serialPort;

    int joyStickAxis[128]; // values: -100 to 100

    void writeBufferToSerialPort(char* buffer, int outLen);
    void processStateChangeMessage(char* ptr, int length);
    void processChartMessage(char* ptr, int length);
    void processReceivedControlMessage(char* ptr, int length);
    void processReceivedStateRequestMessage(char* ptr, int length);
    void processReceivedRawConstantMessage(char *ptr, int length);
    void processReceivedControlParameterMessage(char* ptr, int length);
    void processLoggingControlMessage(char* ptr, int length);
    void processLoggingSizeMessage(char* ptr, int length);

    int16_t extract_2b_from_buffer(char *ptr);
    int32_t extract_4b_from_buffer(char *ptr);
    void processLoggingDumpMessage(char* ptr, int length);

public:
    static DroneController* getInstance(){return instance;}
    QString getLogfile();
protected:
    virtual void  keyPressEvent(QKeyEvent *event);

private slots:
    bool checkChangeMode();
    void readyRead();
    void timerExpired();
    void processJoyStickEvent(int type, int time, int number, int value);
    void initChart();
    void logging_InitChart();
    void on_btnStartLogging_clicked();
    void on_btnStopLogging_clicked();
    void on_btnLogDumpedData_clicked();
    void on_btnPrintLoggedData_clicked();
};



#endif // DRONECONTROLLER_H
