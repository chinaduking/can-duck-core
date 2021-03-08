#include "QOscilloScope.h"
#include "./ui_mainwindow.h"

#include "utils/Tracer.hpp"

QOscilloScope::QOscilloScope(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QOscilloScope)
{
    ui->setupUi(this);
    setWindowTitle(QApplication::translate(
            "QOscilloScope",
            "Elegant OscilloScope       (by S. Dong)", Q_NULLPTR));
}

QOscilloScope::~QOscilloScope()
{
    delete ui;
}

void QOscilloScope::resizeEvent(QResizeEvent * event) {
    float ratio = 0.618;
    ui->scope_widget->resize(event->size().width(), event->size().height() * ratio);

    int bound = 0;
    ui->scope_properties->setGeometry(
            bound, // x
            event->size().height() * ratio + bound, //y,
            event->size().width() - bound * 2,              //w
            event->size().height() * (1.0f-ratio) - bound * 2//h
            );
    //LOGD("resize!");
}