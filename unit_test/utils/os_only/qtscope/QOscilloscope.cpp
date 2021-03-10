//
// Created by 董世谦 on 2021/3/9.
//

#include "utils/os_only/qtscope/QOscilloscope.h"
#include <QApplication>
#include "gtest/gtest.h"
#include "utils/Tracer.hpp"
#include "utils/CppUtils.hpp"
#include <random>
#include <cmath>
using namespace std;

int main(){
    int c = 0;
    QApplication qt_app(c, nullptr);
    QOscilloscope scope;


    auto h0 = scope.addChannel("test 0");
    auto h1 = scope.addChannel("test 1");
    auto h2 = scope.addChannel("test x");
    auto h3 = scope.addChannel("test n");



    double cnt = 0;

    thread data_src([&](){
        while (1){
            h0->addData(sin(cnt) * 2 + (std::rand() % 10) / 20.0 );
            h1->addData(cos(cnt) * 2 + (std::rand() % 10) / 20.0 );
            h2->addData(sin(cnt) * cos(cnt) * 4);
            h3->addData(sin(cnt*cnt));

//            LOGD("added data");
            utils::perciseSleep(0.005);
            cnt += 0.1;
            if(cnt > M_PI * 2){
                cnt = 0;
            }
        }
    });

    scope.show();
    return qt_app.exec();
}
