//
// Created by sdong on 2019/11/11.
//

#ifndef LIBFCN_EVENTLOOPBASE_HPP
#define LIBFCN_EVENTLOOPBASE_HPP

/* 是否使用C++多线程支持。不使用时需要用户定期调用EvLoop::scheduleWorker */
#define EVENTLOOP_THREADING
#define EVENTLOOP_ALLOCATE_DYNAMIC
#define EVENTLOOP_ALLOCATE_STATIC

#include "LinkedList.hpp"

#ifdef EVENTLOOP_THREADING
#include <cstdint>
#include <vector>
#include <thread>
#include <chrono>
#include <condition_variable>
#endif //EVENTLOOP_THREADING

namespace utils {

    enum EventStatus{
        /* just created, or restart() called,
         * waiting for main scheduler first call scheduleWorker()*/
        EVSTAT_WaitingRun,

        /* waiting for notify or timeoutCallback()*/
        EVSTAT_WaitingNofity,

        /* waiting for delete, because of called exit
         * or do not wait notify in scheduleWorker()*/
        EVSTAT_WaitingDelete,
    };

    class EventLoopBase;

    class EventBase{
        friend class EventLoopBase;
    public:
        EventBase();
        virtual ~EventBase();

        enum EventStatus status;
        virtual bool matchNotifyMsg(DataLinkFrame& msg);

        void setStatus(EventStatus status);

        void * operator new(size_t size) noexcept {
            return g_EvloopTaskHeap.getMemBlock(size);
        }

        void operator delete(void * p) {
            g_EvloopTaskHeap.returnMemBlock((uint8_t*)p);
        }

    protected:
        virtual void evUpdate(){}
        virtual void evTimeoutCallback(){}
        virtual void evNotifyCallback(DataLinkFrame& msg){}

        void evWaitNotify(int timeout_ms = -1);
        void evRestart();
        void evExit();

        EventLoopBase* context_evloop;


    private:
        uint64_t timeout_time_us;
    };

    class EventLoopBase{
    public:
        EventLoopBase();
        virtual ~EventLoopBase();


        /* 非阻塞地停止事件循环。事件循环不保证立即响应，
         * 而是会等待当前循环执行完毕才停止，并从线程退出。*/
        void stop();


        /* 阻塞地等待事件循环线程退出，用于在上层析构事件循环
         * 实例之前，等待线程退出。直接删除未退出的线程会引起
         * crash*/
        void spin();


        /* 零拷贝地推入一帧消息 */
        void notify(ESharedPtr<DataLinkFrame>& message);


        /* 更安全的智能指针构造器，构造实例+构造指针+推入数据，
         * 一个API完成。避免先new实例再传参可能造成人为错误。
         * 举例：
         *  evloop.addTask<EventBase>(some_param);
         *
         *  //[ 手动构造的旧方式 ]
         *  // {
         *  //     // 手动构造实例和指向实例的智能指针
         *  //     auto event = ESharedPtr<EventBase>(new EventBase(some_param))
         *  //     // 手动传参
         *  //     evloop.addTask(event);
         *  //     // 智能指针出作用域
         *  // }
         * */
        template< class T, class... Args >
        void addTask( Args&&... args ){
            ESharedPtr<EventBase> sp(new T(args...));
            addTask(sp);
        }


        /* 旧的添加任务方式 */
        void addTask(ESharedPtr<EventBase>& task);

        bool scheduleWorker();
    protected:
        SharedMQ<EventBase> task_pool;
        SharedMQ<DataLinkFrame> notify_queue;

    private:
        void notify();


        int  getMinSleepTime();

        void checkNotify();
        void checkRun();
        void checkDelete();
        void checkTimeout();

        bool is_start;
        bool is_nested_in_evloop;

#ifdef EVENTLOOP_THREADING
        static const int MAX_SLEEP_GAP = 500*1000;
        std::condition_variable sched_ctrl_cv;
        std::mutex update_mutex;
        std::thread* worker_thread;
#endif
    };

}

#endif //LIBFCN_EVENTLOOPBASE_HPP
