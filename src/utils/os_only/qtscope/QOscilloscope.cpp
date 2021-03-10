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

    Theme theme_industrial_dark;
    theme_industrial_dark.background = QColor::fromRgb(100, 100, 100);
    theme_industrial_dark.axis = Qt::white;
    theme_industrial_dark.plots.push_back(Qt::yellow);
    theme_industrial_dark.plots.push_back(Qt::cyan);
    theme_industrial_dark.plots.push_back(Qt::magenta);
    theme_industrial_dark.plots.push_back(Qt::green);
    themes.push_back(theme_industrial_dark);

    Theme theme_light;
    theme_light.background = Qt::white;
    theme_light.axis = Qt::black;
    theme_light.plots.push_back(QColor::fromRgb(159, 140, 220));
    theme_light.plots.push_back(QColor::fromRgb(254, 76,  126));
    theme_light.plots.push_back(QColor::fromRgb(80,  195, 254));
    theme_light.plots.push_back(QColor::fromRgb(126, 221, 64));
    theme_light.plots.push_back(QColor::fromRgb(250, 175, 69));

    themes.push_back(theme_light);


    Theme theme_material_dark;
    theme_material_dark.background = QColor::fromRgb(38, 50, 56);
    theme_material_dark.axis = Qt::white;
    theme_material_dark.plots.push_back(QColor::fromRgb(129, 175, 252));
    theme_material_dark.plots.push_back(QColor::fromRgb(247, 118,  105));
    theme_material_dark.plots.push_back(QColor::fromRgb(195,  232, 135));
    theme_material_dark.plots.push_back(QColor::fromRgb(220, 220, 223));
    theme_material_dark.plots.push_back(QColor::fromRgb(232, 204, 0));
    themes.push_back(theme_material_dark);

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
//    LOGV("paintEvent 0");

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
    w->setBackground(brush);
    w->xAxis->setTickLabelColor(themes[theme_index].axis);
    w->yAxis->setTickLabelColor(themes[theme_index].axis);


    int index = 0;
    for(auto& ch : data_handles){
        QString name(ch.first.c_str());

        auto g = w->graph(index);

        g->setName(name);

        QPen pen;
        pen.setWidth(2);

        if(ch.second->index_buf.size() > 10){
            auto index_buf = ch.second->index_buf;
            auto data_buf = ch.second->data_buf;

//            g->addData(index_buf[index_buf.size()-1],
//                       data_buf[data_buf.size()-1]);

            pen.setColor(themes[theme_index].plots[index]);
            g->setPen(pen);

            //TODO: lock..
            std::unique_lock<std::mutex> lk(ch.second->data_update_mutex);

            g->setData(index_buf, data_buf, true);

            w->xAxis->setRange(ch.second->index_buf[0], index_buf[index_buf.size()-1]);
            w->yAxis->setRange(-5, 5);

            lk.unlock();
        }

//        g->setData(ch.second->index_buf, ch.second->data_buf, true);
        index ++;
    }

    w->legend->setVisible(true);

//    LOGV("paintEvent 1");

    w->replot(QCustomPlot::rpQueuedReplot);

//    LOGV("paintEvent 2");
}


void ScopeChannelHandle::addData(double data){
    std::lock_guard<std::mutex> lk(data_update_mutex);

    int x_data_size = 500;


    switch(trigger_mode){
        case TriggerMode::DataArrive: {
            time_stamp_buf.push_back(utils::getCurrentTimeUs());
            data_buf.push_back(data);
            index_buf.push_back(data_cnt);

            data_cnt += 1;

            if(time_stamp_buf.size() != data_buf.size()){
                time_stamp_buf.clear();
                data_buf.clear();
            }

            if(time_stamp_buf.size() > x_data_size){
                time_stamp_buf.pop_front();
            }

            if(data_buf.size() > x_data_size){
                data_buf.pop_front();
            }


            if(index_buf.size() > x_data_size){
                index_buf.pop_front();
            }
        }
            break;


        case TriggerMode::None:{
            if(data_buf.size() != x_data_size){
                data_buf.resize(x_data_size);
                for(auto& i : data_buf){
                    i = 0;
                }
            }

            if(time_stamp_buf.size() != x_data_size){
                time_stamp_buf.clear();
                time_stamp_buf.resize(x_data_size);
            }

            if(index_buf.size() != x_data_size){
                index_buf.clear();
                index_buf.resize(x_data_size);

                int j = 0;
                for(auto& i : index_buf){
                    i = j;
                    j ++;
                }
            }

            data_buf[trigger_none_data_idx] = data;
            time_stamp_buf[trigger_none_data_idx]  = utils::getCurrentTimeUs();
            index_buf[trigger_none_data_idx] = trigger_none_data_idx;

            trigger_none_data_idx ++;
            if(trigger_none_data_idx >= x_data_size){
                trigger_none_data_idx = 0;
            }
        }
            break;

        default:
            break;
    }


    widget->dataNotify();
}
