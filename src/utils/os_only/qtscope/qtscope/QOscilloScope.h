#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class QOscilloScope; }
QT_END_NAMESPACE

class QOscilloScope : public QMainWindow
{
    Q_OBJECT

public:
    QOscilloScope(QWidget *parent = nullptr);
    ~QOscilloScope();

private:
    Ui::QOscilloScope *ui;

    void resizeEvent(QResizeEvent*) override;
};
#endif // MAINWINDOW_H
