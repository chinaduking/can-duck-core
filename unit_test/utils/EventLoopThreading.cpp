#include "gtest/gtest.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#include "utils/CppUtils.hpp"
#include "utils/EventLoopBase.hpp"
#include "utils/FrameUtils.hpp"

using namespace std;
using namespace emros;
namespace emros_test{
    class TestTask : public EventBase {
    public:
        TestTask(int timeout_ms=1000, string name="") : EventBase() {
            cnt = 0;
            this->timeout_ms = timeout_ms;
            this->name = name;
            prev_callback_time_us = 0;
        }

        void evUpdate() override {
            cout << name <<" -- evUpdate!! " << getCurrentTimeUs() << endl;

            if (cnt < 100) {
                cnt++;
                cout << name <<" ::: call evWaitNotify. " << endl;
                evWaitNotify(timeout_ms);
            } else {
                cout << name <<" ::: call evExit. " << endl;
                evExit();
            }
        }

        void evTimeoutCallback() override {
            cout << name <<" -- evTimeoutCallback!! " << getCurrentTimeUs() << endl;
            cout << name <<" ::: call evRestart. " << endl;
            evRestart();

            uint64_t time_us = getCurrentTimeUs();
            if(prev_callback_time_us == 0){
                prev_callback_time_us = time_us;
                return;
            }

            int time_diff_us = time_us - prev_callback_time_us - timeout_ms * 1000;
            cout << name << " time_diff = " << ((float)time_diff_us) * 0.001 << " ms" << endl;
//            ASSERT_LE(time_diff_us, 1000 * timeout_ms);
            ASSERT_LE(0, time_diff_us);
            prev_callback_time_us = time_us;
        }

        bool matchNotifyMsg(DataLinkFrame& msg) override {
            if(msg.src_id == 1){
                return true;
            }

            return false;
        }

        void evNotifyCallback(DataLinkFrame& msg) override {
            cout << name <<" -- evNotifyCallback!! " << getCurrentTimeUs() << endl;
            cout << "payload: " << DataLinkFrameToString(msg) << endl;
            cout << name <<" ::: call evRestart. " << endl;
            evRestart();
        }


        ~TestTask() {
            cout << name <<" -- task delete!! " << getCurrentTimeUs() << endl;
        }

        int cnt;
        int timeout_ms;
        string name;

        uint64_t prev_callback_time_us;
    };

    TEST(EventLoop, timer) {
        for (int i = 0; i < 3; i++) {
            uint64_t t0 = getCurrentTimeUs();
            sleep(1);
            uint64_t t1 = getCurrentTimeUs();
            ASSERT_NEAR((t1 - t0), 1e6, 2e3); //tolerance: -/+1ms
        }
    }

    TEST(EventLoop, EmptyRun) {
        EventLoopBase evloop;
        sleep(3);
        evloop.stop();
    }

    TEST(EventLoop, TaskTimeout1){
        /* 由于智能指针作用域，会造成内存泄漏。仅供测试使用。 */
        EventLoopBase evloop;
        cout << "-------- time out 1ms test:" << endl;
        ESharedPtr<EventBase> task1(new TestTask(1, "1ms_task"));
        evloop.addTask(task1);
        cout << "-------- time out 10ms test:" << endl;

        ESharedPtr<EventBase> task2(new TestTask(10, "10ms_task"));
        evloop.addTask(task2);

        cout << "-------- time out 100ms test:" << endl;
        ESharedPtr<EventBase> task3(new TestTask(100, "100ms_task"));
        evloop.addTask(task3);

        cout << "-------- time out 1s test:" << endl;
        ESharedPtr<EventBase> task4(new TestTask(1000, "1000ms_task"));

        evloop.addTask(task4);
        sleep(7);

        evloop.stop();
    }


    TEST(EventLoop, TaskTimeoutTemplate){
        EventLoopBase evloop;
        cout << "-------- time out 1ms test:" << endl;
        evloop.addTask<TestTask>(1,    "1ms_task");
        cout << "-------- time out 10ms test:" << endl;
        evloop.addTask<TestTask>(10,   "10ms_task");
        cout << "-------- time out 100ms test:" << endl;
        evloop.addTask<TestTask>(100,  "100ms_task");
        cout << "-------- time out 1s test:" << endl;
        evloop.addTask<TestTask>(1000, "1000ms_task");
        sleep(7);

        evloop.stop();
    }

    TEST(EventLoop, TaskTimeoutShort){
        EventLoopBase evloop;
        evloop.addTask<TestTask>(1,  "1ms_task");
        evloop.addTask<TestTask>(3,  "3ms_task");
        evloop.addTask<TestTask>(4,  "4ms_task");
        evloop.addTask<TestTask>(10, "10ms_task");
        evloop.addTask<TestTask>(7,  "7ms_task");
        evloop.addTask<TestTask>(1,  "1_ms_task");

        sleep(12);
        evloop.stop();
    }

    TEST(EventLoop, Notify1){
        EventLoopBase evloop;
        evloop.addTask<TestTask>(300, "500ms_task");
        sleep(1);

        auto msg = makeESharedPtr<DataLinkFrame>();

        char* payload = "hello world";
        int payload_size = strlen(payload) + 1;
        msg->src_id = 1;

        msg->payload_len = payload_size;
        memcpy(msg->payload, payload, payload_size);

        for(int i = 0; i < 4; i ++){
            cout << "evloop.notify(0x01) 1  " << getCurrentTimeUs() << endl;
            evloop.notify(msg);
            sleep(1);
        }

        cout << "evloop.notify(0x01) 2   " << getCurrentTimeUs() << endl;
        evloop.notify(msg);

        sleep(3);
        evloop.stop();
    }


    TEST(EventLoop, NotifyN){

    }
}



