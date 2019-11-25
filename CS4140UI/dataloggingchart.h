#ifndef DATALOGGINGCHART_H
#define DATALOGGINGCHART_H


#include <QWidget>
#include <QtCharts>
#include <QLineSeries>

namespace Ui {
class DataLoggingChart;
}

class DataLoggingChart : public QWidget
{
    Q_OBJECT

public:
    explicit DataLoggingChart(QWidget *parent = 0);
    ~DataLoggingChart();

    void DataLoggingChart_Init(QStringList seriesNames);

    void DataLoggingChart_AddCharData(QList<int> points);


    QChart *DataLogging_Chart;
    QChartView *DataLogging_ChartView;

private:
    Ui::DataLoggingChart *ui;
};
#endif // DATALOGGINGCHART_H
