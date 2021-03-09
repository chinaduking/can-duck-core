//
// Created by 董世谦 on 2021/3/9.
//

#include "utils/os_only/qtscope/QOscilloscope.h"
#include <QApplication>
#include "gtest/gtest.h"


using namespace std;

TEST(Scope, Plot){
    int c = 0;
    QApplication qt_app(c, nullptr);
    QOscilloscope scope;


    auto h0 = scope.addChannel("test 0");

    thread data_src([&](){
        while (1){
            h0->addData(10);
            printf("added data");
            sleep(1);
        }
    });

    scope.show();
    qt_app.exec();
}
