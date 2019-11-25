#ifndef CHART_H
#define CHART_H

#include <QWidget>
#include <QtCharts>
#include <QLineSeries>


namespace Ui {
class Chart;
}

class Chart : public QWidget
{
    Q_OBJECT

public:
    explicit Chart(QWidget *parent = 0);
    ~Chart();

    void initChart(QStringList seriesNames);

    void addChartData(QList<int> points);


    QChart *chart;
    QChartView *chartView;

private:
    Ui::Chart *ui;
};

#endif // CHART_H
