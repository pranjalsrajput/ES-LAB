#include "chart.h"
#include "ui_chart.h"
#include <QValueAxis>

// by Qt Creator
Chart::Chart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chart)
{
    ui->setupUi(this);
}

// by Qt Creator
Chart::~Chart()
{
    delete ui;
}


// by Beni Kovács
// creates chart with series names received in list
void Chart::initChart(QStringList seriesNames)
{
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

int ix = 0;
int max = 0;
int min = 0;

// by Beni Kovács
// adds a new data point to chart, limited to max 1000
void Chart::addChartData(QList<int> points)
{
    int count = 0;
    ix++;
    for(int i=0; i<chart->series().size(); i++){
        if(points[i]>1000){
            points[i] = 1000;
        }
        QLineSeries *s = static_cast<QLineSeries*>(chart->series()[i]);
        *s << QPointF(ix, points[i]);
        count = s->count();
        if(max < points[i]){
            max = points[i];
        }
        if(min>points[i]){
            min = points[i];
        }
        if(s->count() > 100){
            s->removePoints(0, 1);
        }
    }
    chart->axisX()->setRange(ix-101, ix+1);
    chart->axisY()->setRange(min-10, max+10);
    dynamic_cast<QValueAxis*>(chart->axisY())->applyNiceNumbers();
    chartView->repaint();
}
