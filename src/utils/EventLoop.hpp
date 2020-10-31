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


#ifdef EVENTLOOP_THREADING
#define EVENTLOOP_LOCKGURAD(m_) std::lock_guard<std::mutex> evloop_lock_guard(m_)

    inline uint64_t evloopTimeSrouceMS(){
        return (getCurrentTimeUs() / 1000);
    }
#else
#define EVENTLOOP_LOCKGURAD(m_)
#endif

    template <typename Msg_T, typename ListNodeAlloc=DefaultAllocator>
    class EventLoop{
    public:

        class Task{
            friend class EventLoop;
        public:
            Task()
                : timeout_time_ms(0),
                  status(EVSTAT_WaitingRun),
                  context_evloop(){}

            virtual ~Task() = default;

            enum EventStatus status{EVSTAT_WaitingRun};

            virtual bool matchNotifyMsg(Msg_T& msg) = 0;

            void setStatus(EventStatus status){
                this->status = status;
            }

        protected:
            virtual void evUpdate() = 0;
            virtual void evTimeoutCallback() = 0;
            virtual void evNotifyCallback(Msg_T& msg) = 0;

            void evWaitNotify(int timeout_ms = -1){
                /* 传入非正整数使超时时间为无穷 */
                if(timeout_ms <= 0){
                    timeout_time_ms = 0;
                }else{
                    timeout_time_ms = context_evloop->getCurrentTimeMs() + timeout_ms;
                }

                setStatus(EVSTAT_WaitingNofity);
            }


            void evRestart(){
                /*clear timeout to clear waiting signal state.*/

                this->timeout_time_ms = 0;

                setStatus(EVSTAT_WaitingRun);
            }

            void evExit(){
                setStatus(EVSTAT_WaitingDelete);
            }


            EventLoop* context_evloop{nullptr};




            //TODO: override new /delete using TaskAllocator
        private:
            uint64_t timeout_time_ms{0};
        };


    public:
        EventLoop(uint64_t (*ms_timesource)()):
            ms_timesource(ms_timesource){
#ifdef EVENTLOOP_THREADING
            worker_thread = new std::thread([&](){
                while(scheduleWorker());
            });
#endif
        }

        ~EventLoop() {
            stop();
            /* wait until worker thread stop. force delete
             * worker_thread will cause core dump*/
            spin();

#ifdef EVENTLOOP_THREADING
            delete worker_thread;
#endif

//            std::cout << "EventLoopBase deleted.\n";
        }


        /* 非阻塞地停止事件循环。事件循环不保证立即响应，
         * 而是会等待当前循环执行完毕才停止，并从线程退出。*/
        void stop(){
            EVENTLOOP_LOCKGURAD(update_mutex);

            is_start = false;
        }


        /* 阻塞地等待事件循环线程退出，用于在上层析构事件循环
         * 实例之前，等待线程退出。直接删除未退出的线程会引起
         * crash*/
        void spin(){
            worker_thread->join();
        }



        void notify(Msg_T& message){
            //TODO: 只实现深度为1的通知队列：填入一个消息，并阻塞至所有接收处理完成！
            pending_notify = message;
            notify();
        }


//        /* 更安全的智能指针构造器，构造实例+构造指针+推入数据，
//         * 一个API完成。避免先new实例再传参可能造成人为错误。
//         * 举例：
//         *  evloop.addTask<EventBase>(some_param);
//         *
//         *  //[ 手动构造的旧方式 ]
//         *  // {
//         *  //     // 手动构造实例和指向实例的智能指针
//         *  //     auto event = ESharedPtr<EventBase>(new EventBase(some_param))
//         *  //     // 手动传参
//         *  //     evloop.addTask(event);
//         *  //     // 智能指针出作用域
//         *  // }
//         * */
//        template< class T, class... Args >
//        void addTask( Args&&... args ){
//            ESharedPtr<EventBase> sp(new T(args...));
//            addTask(sp);
//        }


        /* 旧的添加任务方式 */
        void addTask(std::unique_ptr<Task> task){
            task->context_evloop = this;
            task_list.push(task);
        }

        bool scheduleWorker(){
#ifdef EVENTLOOP_THREADING
            std::unique_lock<std::mutex> updating_lk(update_mutex);
#endif
            /*is_nested_in_evloop flag is use for adding task inside scheduleWorker().
             * without it, adding task will cause deadlock!*/
            is_nested_in_evloop = true;

            /*1. check run*/
            checkRun();

            /* cv is notified.
             * there may exist two reasons:
             * notified by new task added,
             * or notified by an event.
             * don't know which task is notified,
             * so let's check it...*/
            checkNotify();

            /* timer is timeout, but don't know which task is timeout,
            * so let's check it...*/
            checkTimeout();

            checkDelete();

#ifdef EVENTLOOP_THREADING
            /* max wait time, that means if all task timeout is longer than 0.5s,
             * or task pool is empty, the scheduler will weak up every 0.5s.
             */
            int min_time_out = getMinSleepTime();

            if(min_time_out > 1){
                /* release updating_lk to avoid deadlock when notify
                 * at main scheduler sleeping.*/
                is_nested_in_evloop = false;
                sched_ctrl_cv.wait_for(
                        updating_lk, std::chrono::microseconds(min_time_out));
                is_nested_in_evloop = true;
            }
#endif
            is_nested_in_evloop = false;

            return is_start;
        }

    protected:

        LinkedList<std::unique_ptr<Task>, ListNodeAlloc> task_list;

    private:
        void notify(){
            //TODO: 只实现深度为1的通知队列：填入一个消息，并阻塞至所有接收处理完成！
#ifdef EVENTLOOP_THREADING
            std::unique_lock<std::mutex> updating_lk(update_mutex);
            sched_ctrl_cv.notify_all();
            //waitComplete();
#endif //EVENTLOOP_THREADING

            is_notify_pending = true;
        }

        Msg_T pending_notify;
        bool is_notify_pending{false};

        int  getMinSleepTime(){

            /* 最大等待时间。当没有任何唤醒/超时事件时，事件循环每0.5s
             * 唤醒自身一次，检查所有任务状态。*/
            /* max wait time, that means if all task timeout is longer than 0.5s,
             * or task pool is empty, the scheduler will weak up every 0.5s.
             */
            int min_time_out = MAX_SLEEP_GAP;

            auto current_time_ms = getCurrentTimeMs();
            for(auto& task : task_list){
                /* 如果有正在等待运行的任务，则返回0直接进入下一个循环。
                 * 一般是因调用了Restart */
                if(task->status == EVSTAT_WaitingRun){
                    min_time_out = 0;
                    break;
                }

                /* 如果没有需要立即重启的任务，
                 * 则在正在等待通知且设置了超时的任务中，搜索下一个最小超时*/
                if(task->status == EVSTAT_WaitingNofity
                   && task->timeout_time_ms != 0) {

                    uint64_t sleep_time = task->timeout_time_ms -
                                          current_time_ms;

                    if (min_time_out > (int) sleep_time) {
                        min_time_out = (int) sleep_time;
                    }
                }
            }

            /* 出现负数的原因是检查超时时，超时的时间点已过，因此返回0直接进入下一个循环。
             * 下一个循环中会调用timeout callback */
            if(min_time_out < 0){
                min_time_out = 0;
            }

            return min_time_out;
        }

        void checkNotify(){
            if(is_notify_pending){
                for (auto& task : task_list) {
                    if (task->matchNotifyMsg(pending_notify)
                        && task->status == EVSTAT_WaitingNofity) {
                        task->setStatus(EVSTAT_WaitingDelete);
                        task->evNotifyCallback(pending_notify);
                    }
                }

                is_notify_pending = false;

//                notifyUnblock();//TODO: 解除notify调用的阻塞
            }
        }

        void checkRun(){
            for(auto& task : task_list) {
                if(task->status == EVSTAT_WaitingRun){
                    /*set task status to delete at first,
                     *if restart() or waitNofify() is called,
                     * task->status will be changed. if neither is
                     * called, task will be deleted*/
                    task->setStatus(EVSTAT_WaitingDelete);
                    task->evUpdate();
                }
            }
        }

        void checkDelete(){
//            while(1){
//                auto node = task_list.find(isWaitingDelete);
//
//                if(node == nullptr){
//                    break;
//                }
//
//                delete node->val;
//                node->val = nullptr;
//
//                task_list.remove(node);
//            }

            task_list.remove_if(isWaitingDelete);
        }

        void checkTimeout(){
            for(auto& task : task_list){
                if(task->status != EVSTAT_WaitingNofity){
                    continue;
                }

                if(task->timeout_time_ms != 0 && getCurrentTimeMs() >
                task->timeout_time_ms){
                    /*same reason as above. set to delete first.*/
                    task->setStatus(EVSTAT_WaitingDelete);
                    task->evTimeoutCallback();
                }
            }
        }

        bool is_start{true};
        bool is_nested_in_evloop{false};

        static bool isWaitingDelete(std::unique_ptr<Task>& task){
            return task->status == EVSTAT_WaitingDelete;
        }

        uint64_t getCurrentTimeMs(){
            if(ms_timesource == nullptr){
                return 0;
            }
            return (*ms_timesource)();
        }

        uint64_t (*ms_timesource)() {nullptr};

#ifdef EVENTLOOP_THREADING
        static const int MAX_SLEEP_GAP = 500*1000;
        std::condition_variable sched_ctrl_cv;
        std::mutex update_mutex;
        std::thread* worker_thread{nullptr};
#endif
    };

}

#endif //LIBFCN_EVENTLOOPBASE_HPP
