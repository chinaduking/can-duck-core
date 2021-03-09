//
// Created by 董世谦 on 2021/3/9.
//

#include "utils/os_only/qtscope/QOscilloscope.h"
#include <QApplication>
#include "gtest/gtest.h"
#include "utils/Tracer.hpp"
#include "utils/CppUtils.hpp"
#include <random>

using namespace std;

TEST(Scope, Plot){
    int c = 0;
    QApplication qt_app(c, nullptr);
    QOscilloscope scope;


    auto h0 = scope.addChannel("test 0");
    auto h1 = scope.addChannel("test 1");

    thread data_src([&](){
        while (1){
            h0->addData(std::rand() % 30 );
            h1->addData(std::rand() % 30 );
            LOGD("added data");
            utils::perciseSleep(0.05);
        }
    });

    scope.show();
    qt_app.exec();
}
