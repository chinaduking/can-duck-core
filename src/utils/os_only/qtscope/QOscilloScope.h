#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "utils/CppUtils.hpp"
QT_BEGIN_NAMESPACE
namespace Ui { class QOscilloScope; }
QT_END_NAMESPACE


class ScopeChannelHandle{
public:

    ScopeChannelHandle(int index, std::string tag="")
        :  tag(tag), index(index){
        time_stamp_buf.reserve(BUFFER_DEPTH);
        val_buf.reserve(BUFFER_DEPTH);
    }

    void addData(double data){
        time_stamp_buf.push_back(utils::getCurrentTimeUs());
        val_buf.push_back(data);

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
    }

    std::string tag;
    int index { -1 };
    int layer { 0 };

    double scale[2] {1, 1};  //x, y
    double offset[2] {0, 0};  //x, y

    QVector<double> time_stamp_buf;
    QVector<double> val_buf;
    static constexpr int BUFFER_DEPTH = 20000; //20k buffer
};


class QOscilloScope : public QMainWindow
{
    Q_OBJECT

public:
    QOscilloScope(QWidget *parent = nullptr);
    ~QOscilloScope();

//    void loadScopeConfig(ScopeConfig* config);

private:
    Ui::QOscilloScope *ui;

    void resizeEvent(QResizeEvent*) override;

    std::map<std::string,
        std::unique_ptr<ScopeChannelHandle>> data_handles;
};


//class QOscilloScope;
//class ScopeConfig{
//public:
//    virtual void AddDict();
//
//    template<typename Prototype>
//    void subscribe(Prototype& proto);
//
//    QOscilloScope* scope;
//};

#endif // MAINWINDOW_H
