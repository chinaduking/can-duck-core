#include "QOscilloscope.h"
#include "./ui_QOscilloScope.h"

#include "utils/Tracer.hpp"
#include "utils/CppUtils.hpp"

using namespace std;

QOscilloscope::QOscilloscope(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QOscilloscope)
{
    ui->setupUi(this);
    setWindowTitle(QApplication::translate(
    "QOscilloscope",
    "Elegant Oscilloscope       (by S. Dong)", Q_NULLPTR));


    timerId = startTimer(100);
}

void QOscilloscope::timerEvent(QTimerEvent *event)
{
    if(data_notified){
        data_notified = false;
        scopeUpdate();
    }
}

QOscilloscope::~QOscilloscope()
{
    delete ui;
}

void QOscilloscope::resizeEvent(QResizeEvent * event) {
    float ratio = 0.618;
    ui->scope_widget->resize(event->size().width(), event->size().height() * ratio);

    int bound = 0;
    ui->scope_properties->setGeometry(
            bound, // x
            event->size().height() * ratio + bound, //y,
            event->size().width() - bound * 2,              //w
            event->size().height() * (1.0f-ratio) - bound * 2//h
            );
    LOGV("scope resized!");

//    ui->scope_widget->update();
}


void QOscilloscope::paintEvent(QPaintEvent* event) {
    scopeUpdate();
}

void QOscilloscope::scopeUpdate(){
    LOGV("paintEvent");

    auto w = ui->scope_widget;
    auto g_cnt = w->graphCount();

    while(g_cnt != data_handles.size()){
        if(g_cnt > data_handles.size()){
            /*remove last one*/
            w->removeGraph(g_cnt-1);
            LOGD("remove a graph..");
        }else if(g_cnt < data_handles.size()){
            w->addGraph();
            LOGD("add a graph..");
        }
        g_cnt = w->graphCount();
    }

    int index = 0;
    for(auto ch : data_handles){
        QString name(ch.first.c_str());

        auto g = w->graph(index);

        g->setName(name);


        QPen pen;
        pen.setWidth(2);
        pen.setColor(Qt::red);
        g->setPen(pen);
        g->setData(ch.second->index_buf, ch.second->data_buf);


        index ++;
    }
    w->rescaleAxes();

    w->legend->setVisible(true);



    ui->scope_widget->repaint();
    w->replot();
}


void ScopeChannelHandle::addData(double data){
    std::lock_guard<std::mutex> lk(data_update_mutex);

    time_stamp_buf.push_back(utils::getCurrentTimeUs());
    data_buf.push_back(data);
    index_buf.push_back(data_cnt);

    data_cnt += 1;

    if(time_stamp_buf.size() != data_buf.size()){
        time_stamp_buf.clear();
        data_buf.clear();
    }

    if(time_stamp_buf.size() > 200){
        time_stamp_buf.pop_front();
    }

    if(data_buf.size() > 200){
        data_buf.pop_front();
    }


    if(index_buf.size() > 200){
        index_buf.pop_front();
    }

    widget->dataNotify();
}
