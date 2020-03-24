#include "ransacwidget.h"
#include "ui_ransacwidget.h"

RanSacWidget::RanSacWidget(QWidget *parent) :
    QWidget(parent),
    m_isGraph(false),
    m_in_lier_thresh(100),  // generate sample threshold
    m_out_lier_thresh(300), // generate sample threshold
    m_minValue(-300),
    m_maxValue(300),
    ui(new Ui::RanSacWidget)
{
    ui->setupUi(this);
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec()); // seed
    m_series_in = new QScatterSeries();
    m_series_out = new QScatterSeries();
    m_seriesL = new QLineSeries();
    m_seriesR = new QLineSeries();
    m_chart = new QChart();
    m_chartView= new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chart->setTitle("Generated Samples");
    m_axisX = new QValueAxis;
    m_axisY = new QValueAxis;
    connect(ui->generateButton, SIGNAL(clicked()), this, SLOT(generateSample()));
    connect(ui->ranSacButton, SIGNAL(clicked()), this, SLOT(ransacButtonClicked()));
}

RanSacWidget::~RanSacWidget()
{
    if (m_chart)
        delete m_chart;
    if (m_series_in)
        delete m_series_in;
    if (m_series_out)
        delete m_series_out;
    if (m_seriesR)
        delete m_seriesR;
    if (m_seriesL)
        delete m_seriesL;
    if (m_chartView)
        delete m_chartView;
    if (m_axisY)
        delete m_axisY;
    if (m_axisX)
        delete m_axisX;
    delete ui;
}

int RanSacWidget::randInt(int low, int high){
    return qrand() % ((high + 1) - low) + low;
}

bool RanSacWidget::throwInLierNumber(int distance){
    int x = randInt(-300, 300);
    int y = randInt(-300, 300);
    if(pList_all.indexOf(QPoint(x, y)) == -1){ //
        if(fabs(x * m_a-y + m_b)/sqrt(m_a*m_a+1) < distance){
            pList.append(QPoint(x,y));
            pList_all.append(QPoint(x,y));
            return true;
        }
    }
    return false;
}

bool RanSacWidget::throwOutLierNumber(int distance){
    int x = randInt(-300, 300);
    int y = randInt(-300, 300);
    if(pList_all.indexOf(QPoint(x, y)) == -1) { //
        if(fabs(x * m_a-y + m_b)/sqrt(m_a*m_a+1) > distance){
            pList_out.append(QPoint(x,y));
            pList_all.append(QPoint(x,y));
            return true;
        }
    }
    return false;
}

double RanSacWidget::distance_two_points(QPoint &p1, QPoint &p2)
{
     return sqrt((p2.x() - p1.x()) * (p2.x() - p1.x()) + (p2.y() - p1.y()) * (p2.y() - p1.y()));
}


double RanSacWidget::cross_product(QPoint&p1, QPoint&p2){
    return p1.x()*p2.y() - p2.x()*p1.y();
}
// p1 : A
// p2 : B
// p3 : C
// p1 line start // p2 line end
double RanSacWidget::distanceDotToLine(QPoint p1, QPoint p2, QPoint p3){
    double AB_x = p2.x()-p1.x();
    double AB_y = p2.y()-p1.y();
    double AC_x = p3.x()-p1.x();
    double AC_y = p3.y()-p1.y();
    QPoint AB(AB_x, AB_y);
    QPoint AC(AC_x, AC_y);
    return fabs(cross_product(AB, AC)/ distance_two_points(p1,p2));
}

void RanSacWidget::ransacEstimate(){
      int n = stof(ui->nEdit->text().toStdString()); // 샘플링하는 점
      int k = stoi(ui->kEdit->text().toStdString()); //
      int t = stoi(ui->tEdit->text().toStdString()); // 거리 안에 것들
      int d = stoi(ui->dEdit->text().toStdString()); // 많족되는 점이 몇개이상
      // ransac Algorithm
      // k = log(1-p)/log(1-w^n)
      float p=0.99;
      int k_require = log(1-p)/log(1-pow(m_w,n));
      // total point
      int iter=0;
      int bestInlier=0;
      int inlier=0;
      QPoint p1,p2; //to save best samples
      while(iter < k){ // 랜덤하게 두 점 뽑음
          int r1 =  qrand()%(m_total_samples/2);
          int r2 = (qrand()%(m_total_samples/2))+r1;
          // 두 점을 제외한 다른 좌표들과의 distance 구함
          for(int i=0; i<pList_all.size(); i++){
              if(i!=r1 && i!=r2 ){
                //distance r1, r2와 i의 거리
                double distance = distanceDotToLine(pList_all.at(r1), pList_all.at(r2), pList_all.at(i));
                if (t > distance) // counting inlier number
                    inlier++;
              }
          }
          if(inlier>bestInlier) {
              bestInlier=inlier;
              p1=pList_all.at(r1);
              p2=pList_all.at(r2);
           }
          if(bestInlier > d)
              break;
          inlier=0;
          iter++;
      }
      m_seriesR->clear();
      m_seriesR->append(p1.x(), p1.y());
      m_seriesR->append(p2.x(), p2.y());
      m_seriesR->setName("RanSac line"); // line 끝점 구하기
      QString q;
      q.sprintf("[Log] max_iters: %d, Point1 : (%d, %d), Point2 : (%d, %d), BestInlierNum : %d", k, p1.x(), p1.y(), p2.x(), p2.y(), bestInlier);
      ui->textEdit->append(q);
}


void RanSacWidget::ransacButtonClicked(){

    if(m_isGraph) {
    // try
        ransacEstimate();
        m_chartView->update();
    } else {
    // error Box
        QMessageBox msgBox;
        msgBox.setText("[ Data not load ]");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
    }
}

// w는 outlier 비율 > n은 전체 샘플 갯수
void RanSacWidget::randomGenerate(int n, float w){
    // y = ax+b
    // a random
    // -m_maxValue/2 < b < m_maxValue/2
    // 1. line initial
    // * bias initialize        -> b
    // * inclination initialize -> a
    m_b = randInt(0, m_maxValue) - m_maxValue/2;
    m_total_samples = n;
    m_a = ((qrand()%10)+1)/10.;
    int w_num=w*m_total_samples;
    int o_num=m_total_samples - w_num;
    int cnt=0;

    // generate Inlier and Outlier
    while(cnt < w_num){
        if (throwInLierNumber(m_in_lier_thresh))
            cnt++;
    }

    cnt=0;
    while(cnt < o_num){
        if(throwOutLierNumber(m_out_lier_thresh))
            cnt++;
    }

    Eigen::MatrixXd A(m_total_samples, 2);
    Eigen::MatrixXd B(m_total_samples, 1);
    // create matrix
    for(int i=0; i<100; i++) {
            A(i,0) = pList_all.at(i).x();
            A(i,1) = 1.;
            B(i,0) = pList_all.at(i).y();
    }
    // stage 3 > LMS
    Eigen::MatrixXd f(2, 1);
    f = (A.transpose()*A).inverse() * A.transpose() * B;
    m_seriesL->append(m_minValue, m_minValue*f(0)+f(1));
    m_seriesL->append(m_maxValue, m_maxValue*f(0)+f(1));
    m_seriesL->setColor(QColor(255,0,0));
    m_seriesL->setName("All Points LMS line(with outlier)"); // line 끝점 구하기

}

void RanSacWidget::setUpAxis(int min, int max)
{
    m_axisX->setRange(min, max);
    m_axisX->setTickCount(6);
    m_axisX->setLabelFormat("%i");
    m_axisX->setTitleText("X Point");
    m_axisY->setRange(min, max);
    m_axisY->setTickCount(6);
    m_axisY->setLabelFormat("%i");
    m_axisY->setTitleText("Y Point");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series_in->attachAxis(m_axisX); // attachAxis 해줘야 제대로 보인다.
    m_series_in->attachAxis(m_axisY);
    m_series_out->attachAxis(m_axisX); // attachAxis 해줘야 제대로 보인다.
    m_series_out->attachAxis(m_axisY);
    m_seriesL->attachAxis(m_axisX); // attachAxis 해줘야 제대로 보인다.
    m_seriesL->attachAxis(m_axisY);
    m_seriesR->attachAxis(m_axisX); // attachAxis 해줘야 제대로 보인다.
    m_seriesR->attachAxis(m_axisY);
}

void RanSacWidget::generateSample()
{

    float w = stof(ui->wEdit->text().toStdString());
    int   tp= stoi(ui->tpEdit->text().toStdString());
    m_w=w;
    QList<QPoint>::iterator i;
    if (!m_isGraph){
        randomGenerate(tp, w);
        for (i = pList.begin(); i != pList.end(); ++i)
            m_series_in->append((*i).rx(), (*i).ry());
        for (i = pList_out.begin(); i != pList_out.end(); ++i)
            m_series_out->append((*i).rx(), (*i).ry());

        m_series_in->setMarkerSize(5);
        m_series_in->setName("Inlier Data Points");
        m_series_in->setColor(QColor(102,255,0));
        m_series_out->setColor(QColor(255,102,51));
        m_series_out->setMarkerSize(5);
        m_series_out->setName("Outlier Data Points");

        m_chart->addSeries(m_series_in);
        m_chart->addSeries(m_series_out);
        m_chart->addSeries(m_seriesL);
        m_chart->addSeries(m_seriesR);
        setUpAxis(m_minValue, m_maxValue);
        ui->gridLayout->addWidget(m_chartView);
        m_isGraph = true;

    } else {
        pList.clear();
        pList_out.clear();
        pList_all.clear();
        m_chart->removeAllSeries(); // series object delete
        m_seriesL = new QLineSeries();
        m_series_in = new QScatterSeries();
        m_series_out = new QScatterSeries();
        m_series_in->setColor(QColor(102,255,0));
        m_series_out->setColor(QColor(255,102,51));
        randomGenerate(tp, w);
        m_series_in->setMarkerSize(5);
        m_series_in->setName("Inlier Data Points");
        m_series_out->setMarkerSize(5);
        m_series_out->setName("Outlier Data Points");
        for (i = pList.begin(); i != pList.end(); ++i)
            m_series_in->append((*i).rx(), (*i).ry());
        for (i = pList_out.begin(); i != pList_out.end(); ++i)
            m_series_out->append((*i).rx(), (*i).ry());
        m_chart->addSeries(m_series_in);
        m_chart->addSeries(m_series_out);
        m_chart->addSeries(m_seriesL);
        m_chartView->update(); // 변화를 감지하고 update
    }

}


