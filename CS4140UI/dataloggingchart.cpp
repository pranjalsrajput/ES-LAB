#include "dataloggingchart.h"
#include "ui_chart.h"
#include <QValueAxis>


DataLoggingChart::DataLoggingChart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataLoggingChart)
{
    ui->setupUi(this);
}

DataLoggingChart::~DataLoggingChart(){
    delete ui;
}

void DataLoggingChart::DataLoggingChart_Init(QStringList seriesNames){
    chart = new QChart();
    for(int i=0; i<seriesNames.size(); i++){
        QLineSeries *s = new QLineSeries();
        s->setName(seriesNames[i]);
        chart->addSeries(s);
    }
    chart->createDefaultAxes();
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    this->ui->mainLayout->addWidget(chartView);
    chart->setTitle("Drone");
}

void DataLoggingChart::DataLoggingChart_AddCharData(QList<int> points){

}
