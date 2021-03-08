#include "QOscilloScope.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QOscilloScope w;
    w.show();

//    MainWindow w2;
//    w2.show();
    return a.exec();
}
