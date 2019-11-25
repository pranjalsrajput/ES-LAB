#include "dronecontroller.h"
#include "ui_dronecontroller.h"
#include <QDebug>
#include <QTimer>
#include "chart.h"
#include "loggingChart.h"
#include "com_cpp.h"
#include <QMediaPlayer>

DroneController* DroneController::instance;

QTimer *timer;

// by Beni Kovács
DroneController::DroneController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DroneController)
{
    ui->setupUi(this);
    instance = this;

    initChart();

    if(!initSerialPort()){
        qDebug()<<"Flight control boards error";
    }

    initJoyStick();

    timer = new QTimer();
#ifndef DRONE_DEBUG
    timer->setInterval(30);
#else
    timer->setInterval(100);
#endif
    connect(timer, SIGNAL(timeout()), this, SLOT(timerExpired()));
    timer->start();


    newState.mode = 's';
    newState.liftKeyboard = 0;
    newState.rollKeyboard = 0;
    newState.pitchKeyboard = 0;
    newState.yawKeyboard = 0;
    newState.controllerPYawKeyboard = 0;
    newState.controllerP1RollPitchKeyboard = 0;
    newState.controllerP2RollPitchKeyboard = 0;

}

// by Beni Kovács
DroneController::~DroneController()
{
    delete ui;
}

// by Beni Kovács
void DroneController::initChart()
{
    QStringList titles;
    titles << "Motor1" << "Motor2" << "Motor3" << "Motor4"<<"sr non filter"<<"sr filtered";
    chartView = new Chart();
    chartView->initChart(titles);
    chartView->show();
}

/**
 * logging_InitChart. It initialize the chart with the currents names of the data and read the values from the log file to plot them.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
void DroneController::logging_InitChart(){
    QStringList titles;

    titles << "phi" << "theta" << "psi" << "tsp" << "tsq" << "sr" << "sr" << "sax" << "say" << "saz" << "bat_volt" << "pressure" << "tempeture" << "motor1" << "motor2" << "motor3" << "motor4";
    loggingChartView = new LoggingChart();
    loggingChartView->LoggingChart_init(titles);
    loggingChartView->show();
    loggingChartView->LoggingChart_ReadFile();
    //loggingChartView->LoggingChart_addData();
}

/**
 * getLogfile. It return the logfile path.
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return QString. The logfile path.
 * */
QString DroneController::getLogfile(){

    return logfile_name;
}

// @author Stefanos Koffas
void DroneController::on_btnStartLogging_clicked()
{
    newState.logging_state = 's';
}

// @author Stefanos Koffas
void DroneController::on_btnStopLogging_clicked()
{
    newState.logging_state = 'x';
}

// @author Stefanos Koffas
void DroneController::on_btnLogDumpedData_clicked()
{
    newState.logging_size_request = true;
}

/**
 * on_btnPrintLoggedData_clicked. It initialize the chart with a series of values to plot.
 * @author Miguel Pérez Ávila.
 * @param  QStrinlgList seriesNames. The names of the data to be printed.
 * @return void.
 * */
void DroneController::on_btnPrintLoggedData_clicked(){
    logging_InitChart();
}

//just for fun, play sound on mode change
// by Beni Kovács
void DroneController::playSound(QString name){
    QMediaPlayer *player = new QMediaPlayer();
    QString a = QCoreApplication::applicationDirPath();
    if(!a.endsWith("/")){
        a+="/";
    }
    a = a+"sound/"+name+".mp3";
    player->setMedia(QUrl::fromLocalFile(a));
    player->setVolume(50);
    player->play();
}
