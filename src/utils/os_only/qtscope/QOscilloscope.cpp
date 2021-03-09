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


    data_upd_check_th = new std::thread([&](){
        while (1){
            if(data_notified){
                data_notified = false;
                ui->scope_widget->repaint();
                LOGV("data_upd_check_th: cope_widget->repaint");
            }
            utils::perciseSleep(0.02); //50Hz update max
        }
    });
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
    LOGV("paintEvent");
}

void ScopeChannelHandle::addData(double data){
    std::lock_guard<std::mutex> lk(data_update_mutex);

    time_stamp_buf.push_back(utils::getCurrentTimeUs());
    val_buf.push_back(data);
    index_buf.push_back(data_cnt);

    data_cnt += 1;

    if(time_stamp_buf.size() != val_buf.size()){
        time_stamp_buf.clear();
        val_buf.clear();
    }

    if(time_stamp_buf.size() > 200){
        time_stamp_buf.pop_front();
    }

    if(val_buf.size() > 200){
        val_buf.pop_front();
    }

    widget->dataNotify();
}
