#ifndef LOGGINGCHART_H
#define LOGGINGCHART_H

#include <QWidget>
#include <QtCharts>
#include <QLineSeries>
/*Miguel*/

namespace Ui {
class LoggingChart;
}

class LoggingChart : public QWidget
{
    Q_OBJECT

public:
    explicit LoggingChart(QWidget *parent = 0);
    ~LoggingChart();

    void LoggingChart_init(QStringList seriesNames);


    void LoggingChart_ReadFile();

    QChart *loggingChart;
    QChartView *loggingChartView;

private:
    Ui::LoggingChart *ui_LoggingChart;

    void LoggingChart_addData(QList<int> points);

};

#endif // CHART_H
