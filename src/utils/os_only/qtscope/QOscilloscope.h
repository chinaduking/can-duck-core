#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "utils/CppUtils.hpp"
#include <thread>
QT_BEGIN_NAMESPACE
namespace Ui { class QOscilloscope; }
QT_END_NAMESPACE

class QOscilloscope;

class ScopeChannelHandle{
public:

    enum class TriggerMode{
        None, /* 无触发，为新数据覆盖旧数据的滚动效果，适合观测高速周期信号*/
        DataArrive, /* 新数据到来触发，为整体滚动显示效果，适合观测低速非周期信号*/
        RiseLatch, /* 检测到上升沿后锁存，适合观测高速非周期信号*/
    } trigger_mode{TriggerMode::None};

    ScopeChannelHandle(QOscilloscope* widget, int index, std::string tag="")
        :  tag(tag), index(index), widget(widget){
        time_stamp_buf.reserve(BUFFER_DEPTH);
        data_buf.reserve(BUFFER_DEPTH);
    }

    void addData(double data);

    std::string tag;
    int index { -1 };
    int layer { 0 };

    double scale[2] {1, 1};  //x, y
    double offset[2] {0, 0};  //x, y

    uint64_t data_cnt {0};

    QVector<double> index_buf;
    QVector<double> time_stamp_buf;
    QVector<double> data_buf;
    static constexpr int BUFFER_DEPTH = 20000; //20k buffer

    std::mutex data_update_mutex;
    int trigger_none_data_idx{0};

private:
    QOscilloscope* const widget;



//    double getData(){
//        std::lock_guard<std::mutex> lk(data_update_mutex);
//        return
//    }
};


class QOscilloscope : public QMainWindow
{
    Q_OBJECT

public:
    QOscilloscope(QWidget *parent = nullptr);
    ~QOscilloscope();

//    void loadScopeConfig(ScopeConfig* config);

    inline void dataNotify(){
        data_notified = true;
    }

    inline std::shared_ptr<ScopeChannelHandle> addChannel(std::string tag){
        auto p_handle =
                std::make_shared<ScopeChannelHandle>(this, 0, tag);

        data_handles.emplace(tag, p_handle);

        return p_handle;
    }

private:
    Ui::QOscilloscope  *ui;

    void resizeEvent(QResizeEvent*) override;

    void scopeUpdate();

    std::map<std::string,
        std::shared_ptr<ScopeChannelHandle>> data_handles;

    bool data_notified {false};

    int timerId;

    struct Theme{
        QColor background;
        QColor axis;
        QVector<QColor> plots;
    };

    QVector<Theme> themes;
    int theme_index{1};


protected:
    void timerEvent(QTimerEvent *event);


};


//class QOscilloscope;
//class ScopeConfig{
//public:
//    virtual void AddDict();
//
//    template<typename Prototype>
//    void subscribe(Prototype& proto);
//
//    QOscilloscope* scope;
//};

#endif // MAINWINDOW_H
