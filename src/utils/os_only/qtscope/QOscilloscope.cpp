#include "QOscilloscope.h"
#include "./ui_QOscilloscope.h"

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


    timerId = startTimer(50);
    /*使能openGl在视网膜显示器造成尺度不对*/
    // ui->scope_widget->setOpenGl(true, 1);

    Theme theme_dark;
    theme_dark.background = QColor::fromRgb(160,160,160);
    theme_dark.axis = Qt::white;
    theme_dark.plots.push_back(Qt::yellow);
    theme_dark.plots.push_back(Qt::cyan);
    theme_dark.plots.push_back(Qt::magenta);
    theme_dark.plots.push_back(Qt::green);
    themes.push_back(theme_dark);

    Theme theme_light;
    theme_light.background = Qt::white;
    theme_dark.axis = Qt::black;
    theme_light.plots.push_back(QColor::fromRgb(220, 180, 180));
    theme_light.plots.push_back(QColor::fromRgb(160, 160, 255));
    theme_light.plots.push_back(Qt::black);
    theme_light.plots.push_back(Qt::green);
    themes.push_back(theme_light);

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

void QOscilloscope::scopeUpdate(){
    LOGV("paintEvent 0");

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

    QBrush brush(themes[theme_index].background);
//    brush.setColor();
    w->setBackground(brush);
//        w->xAxis->setLabelColor(Qt::white);
    w->xAxis->setTickLabelColor(themes[theme_index].axis);
//        w->xAxis->setLabelColor(Qt::white);
    w->yAxis->setTickLabelColor(themes[theme_index].axis);


    int index = 0;
    for(auto ch : data_handles){
        QString name(ch.first.c_str());

        auto g = w->graph(index);

        g->setName(name);

        QPen pen;
        pen.setWidth(1);

        if(ch.second->index_buf.size() > 10){
            auto index_buf = ch.second->index_buf;
            auto data_buf = ch.second->data_buf;

//            g->addData(index_buf[index_buf.size()-1],
//                       data_buf[data_buf.size()-1]);

            pen.setColor(themes[theme_index].plots[index]);
            g->setPen(pen);

            g->setData(index_buf, data_buf, true);

            w->xAxis->setRange(ch.second->index_buf[0], index_buf[index_buf.size()-1]);
            w->yAxis->setRange(-5, 5);
        }

//        g->setData(ch.second->index_buf, ch.second->data_buf, true);
        index ++;
    }

    LOGV("paintEvent 1");

    w->replot(QCustomPlot::rpQueuedReplot);

    LOGV("paintEvent 2");
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

    if(time_stamp_buf.size() > 500){
        time_stamp_buf.pop_front();
    }

    if(data_buf.size() > 500){
        data_buf.pop_front();
    }


    if(index_buf.size() > 500){
        index_buf.pop_front();
    }

    widget->dataNotify();
}
