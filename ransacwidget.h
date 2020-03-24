#ifndef RANSACWIDGET_H
#define RANSACWIDGET_H
#include <iostream>
#include <QPoint>
#include <QWidget>
#include <QImage>             //
#include <QPainter>           //
#include <QtMath>             //
#include <QtCharts/QChartView>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <QWidget>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QTime>
#include <QChartView>
#include <QMessageBox>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QLegend>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QApplication>          // Eigen
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace std;
using namespace QtCharts;// QCharts를 쓸려면 namespace를 지정해주기
using namespace cv;

namespace Ui {
class RanSacWidget;
}

class RanSacWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RanSacWidget(QWidget *parent = nullptr);
    void randomGenerate(int n, float w); // n=100
    void setUpAxis(int min, int max);
    int randInt(int low, int high);
    bool throwInLierNumber(int distance);
    bool throwOutLierNumber(int distance);
    double distanceDotToLine(QPoint p1, QPoint p2, QPoint p3);
    double distance_two_points(QPoint &p1, QPoint &p2);
    double cross_product(QPoint&p1, QPoint&p2);
    void ransacEstimate();
    ~RanSacWidget();

private:
    // graph min, max range
    int m_isGraph;              // 그래프가 띄워져 있으면 true, 그렇지 않으면 false
    int m_minValue;
    int m_maxValue;
    int m_total_samples;
    int m_in_lier_thresh;
    int m_out_lier_thresh;
    // number generate line parameter
    float m_a;
    int m_b;
    float m_w;

    Ui::RanSacWidget *ui;

    // pointer List
    QList<QPoint> pList;
    QList<QPoint> pList_out;
    QList<QPoint> pList_all;

    // for graph
    QScatterSeries * m_series_in;   // inlier points
    QScatterSeries * m_series_out;  // outlier points
    QLineSeries * m_seriesL;        // save all point Line info
    QLineSeries * m_seriesR;        // save Ransac Line info
    QChartView * m_chartView;       //
    QChart * m_chart;               //
    QValueAxis *m_axisX;            //
    QValueAxis *m_axisY;            //


private slots:
    void generateSample();
    void ransacButtonClicked();

};

#endif // RANSACWIDGET_H
