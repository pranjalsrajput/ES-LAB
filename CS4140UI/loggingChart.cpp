#include "loggingChart.h"
#include "ui_loggingChart.h"
#include <QValueAxis>
#include "dronecontroller.h"
/**
 * LoggingChart. Constructor of the class. It creates the chart and setup the UI.
 * @author Miguel Pérez Ávila.
 * @param  QWidget *parent. The QWidget parent.
 * @return void.
 * */
LoggingChart::LoggingChart(QWidget *parent) :
    QWidget(parent),
    ui_LoggingChart(new Ui::LoggingChart)
{
    ui_LoggingChart->setupUi(this);
}
/**
 * ~LoggingChart. It deletes the chart. 
 * @author Miguel Pérez Ávila.
 * @param  none.
 * @return void.
 * */
LoggingChart::~LoggingChart()
{
    delete ui_LoggingChart;
}

/**
 * LoggingChart_init. It initialize the chart with a series of values to plot.
 * @author Miguel Pérez Ávila.
 * @param  QStrinlgList seriesNames. The names of the data to be printed.
 * @return void.
 * */
void LoggingChart::LoggingChart_init(QStringList seriesNames)
{
    loggingChart = new QChart();
    for(int i=0; i<seriesNames.size(); i++){
        QLineSeries *s = new QLineSeries();
        s->setName(seriesNames[i]);
        loggingChart->addSeries(s);
    }
    loggingChart->createDefaultAxes();
    loggingChartView = new QChartView(loggingChart);
    loggingChartView->setRenderHint(QPainter::Antialiasing);
    this->ui_LoggingChart->mainLayout->addWidget(loggingChartView);
    loggingChart->setTitle("Data Logging");
}

int min_x = 0;
int max_x = 1;
int max_y = 0;
int min_y = 0;


/**
 * LoggingChart_ReadFile. It reads the log file line by line all the useful data of the log file and plot it in the chart.
 * @author Miguel Pérez Ávila.
 * @param none.
 * @return void.
 * */
void LoggingChart::LoggingChart_ReadFile(){
    QFile file(DroneController::getInstance()->getLogfile());
    file.open(QIODevice::ReadOnly);

    QTextStream in(&file);
    bool firstLine = true;
    //in.readLine();
    while(!in.atEnd()) {

       QString line = in.readLine();
       QStringList  fields = line.split(",");
       QList<int> measures;
       //fields.removeFirst();
       //fields.removeLast();
       int fieldsLength = fields.length();
       for( int i = 0; i < fieldsLength; i++){

            if(i == 0){
               QString  first = fields.takeFirst();
               first.remove(0,1);
               measures.append(first.toInt());
               if(firstLine == true){

                   min_x = measures[0];
                   firstLine = false;
               }
               if(measures[0] > max_x){
                   max_x = measures[0];
               }
            }else{
               measures.append(fields.takeFirst().toInt());
            }

       }
       //QLineSeries *serie =
       //QLineSeries *series = new QLineSeries();
       this->LoggingChart_addData(measures);
    }
}
/**
 * LoggingChart_addData Plot all the points according to the first value of the list that is the time.
 * @author Miguel Pérez Ávila.
 * @param  QList<int> points. A list of points to be plot in the graph.
 * @return void.
 * */
void LoggingChart::LoggingChart_addData(QList<int> points)
{
    for(int i=1; i<loggingChart->series().size(); i++){
        QLineSeries *s = static_cast<QLineSeries*>(loggingChart->series()[i]);
        *s << QPointF(points[0], points[i]);
        if(max_y < points[i]){
            max_y = points[i];
        }
        if(min_y>points[i]){
            min_y = points[i];
        }

    }
    loggingChart->axisX()->setRange(min_x, max_x);
    loggingChart->axisY()->setRange(min_y, max_y);
    dynamic_cast<QValueAxis*>(loggingChart->axisY())->applyNiceNumbers();
    loggingChartView->repaint();
}

