//
// Created by sdong on 2019/11/11.
//

#ifndef LIBFCN_EVENTLOOPBASE_HPP
#define LIBFCN_EVENTLOOPBASE_HPP

/* 是否使用C++多线程支持。不使用时需要用户定期调用EvLoop::scheduleWorker */
#ifdef SYSTYPE_FULL_OS
#define EVENTLOOP_THREADING
#endif
#define EVENTLOOP_ALLOCATE_DYNAMIC
#define EVENTLOOP_ALLOCATE_STATIC

#include "LinkedList.hpp"

#ifdef EVENTLOOP_THREADING
#include <cstdint>
#include <vector>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <iostream>
#endif //EVENTLOOP_THREADING

namespace utils {



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

        enum class TaskState : uint8_t {
            /* just created, or restart() called,
             * waiting for main scheduler first call scheduleWorker()*/
            WaitingRun,

            /* waiting for notify or timeoutCallback()*/
            WaitingNotify,

            /* waiting for delete, because of called exit
             * or do not wait notify in scheduleWorker()*/
            WaitingDelete,
        };


        class Task{
            friend class EventLoop;
        public:
            Task()
                : timeout_time_ms(0),
                  status(TaskState::WaitingRun){}

            virtual ~Task() = default;

            TaskState status{TaskState::WaitingRun};

            virtual bool matchNotifyMsg(Msg_T& msg) = 0;

            void setStatus(TaskState status){
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

                setStatus(TaskState::WaitingNotify);
            }


            void evRestart(){
                /*clear timeout to clear waiting signal state.*/

                this->timeout_time_ms = 0;

                setStatus(TaskState::WaitingRun);
            }

            void evExit(){
                setStatus(TaskState::WaitingDelete);
            }


            EventLoop* context_evloop{nullptr};

        private:
            uint64_t timeout_time_ms{0};
        };


    public:
        EventLoop(uint64_t (*ms_timesource)(), uint16_t task_limit = 0):
            ms_timesource(ms_timesource),
            task_limit(task_limit){
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



        /* 添加任务 */
        int addTask(std::unique_ptr<Task> task){
            if(task == nullptr){
                return -1;
            }

            if((task_limit != 0 && task_list.size() < task_limit)
                || task_limit == 0){
                task->context_evloop = this;
                int res = task_list.push(task);
                /* 如果当前调度器正处于休眠模式，添加任务可唤醒一次循环 */
                sched_ctrl_cv.notify_all();

                if(res == -1){
                    return -1;
                }

                return task_list.size() - 1;
            }
            return -1;
        }

        void notify(Msg_T& message){
//            std::cout << "evloop::notify!!!" <<std::endl;

            //目前只实现深度为1的通知队列：填入一个消息，并阻塞至所有接收处理完成！
#ifdef EVENTLOOP_THREADING
            std::unique_lock<std::mutex> updating_lk(update_mutex);
            //waitComplete();
#endif //EVENTLOOP_THREADING


            USER_ASSERT(!is_notify_pending);

            is_notify_pending = true;
            pending_notify = message;
#ifdef EVENTLOOP_THREADING
            sched_ctrl_cv.notify_all();
            notify_ctrl_cv.wait(updating_lk);
//            std::cout << "notify_ctrl_cv done!!!" <<std::endl;
#endif //EVENTLOOP_THREADING
        }



        bool scheduleWorker(){
#ifdef EVENTLOOP_THREADING
            std::unique_lock<std::mutex> updating_lk(update_mutex);
#endif
//            std::cout << "evloop::scheduleWorker!!!" <<std::endl;

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
            /* max wait time, that means if all task timeout is longer than 1s,
             * or task pool is empty, the scheduler will weak up every 1s.
             */
            int min_time_out = getMinSleepTime();
//            std::cout << "sleep ms = " << min_time_out << std::endl;
            if(min_time_out > 1){
                /* release updating_lk to avoid deadlock when notify
                 * at main scheduler sleeping.*/
                is_nested_in_evloop = false;
                auto cv_ret = sched_ctrl_cv.wait_for(  //TODO: ms??
                        updating_lk, std::chrono::microseconds
                        (min_time_out*1000));

//                if(cv_ret == std::cv_status::timeout){
//                    std::cout << "sched_ctrl_cv is timeout! " << min_time_out
//                    << std::endl;
//                }else if(cv_ret == std::cv_status::no_timeout){
//                    std::cout << "sched_ctrl_cv is NOT timeout! " <<
//                    min_time_out
//                              << std::endl;
//                }

                is_nested_in_evloop = true;
            }
//            else{
//                  //release cpu for a while
//                is_nested_in_evloop = false;
//                sched_ctrl_cv.wait_for(
//                        updating_lk, std::chrono::milliseconds (1));
//                is_nested_in_evloop = true;
//            }
#endif
            is_nested_in_evloop = false;

            return is_start;
        }

    protected:

        LinkedList<std::unique_ptr<Task>, ListNodeAlloc> task_list;
        uint16_t task_limit{0};
    private:
//        void notify(){
//
//        }

        Msg_T pending_notify;
        bool is_notify_pending{false};

        int  getMinSleepTime(){

            /* 最大等待时间。当没有任何唤醒/超时事件时，事件循环每1s
             * 唤醒自身一次，检查所有任务状态。*/
            /* max wait time, that means if all task timeout is longer than 1s,
             * or task pool is empty, the scheduler will weak up every 1s.
             */
            int min_time_out = MAX_SLEEP_GAP_MS;

            auto current_time_ms = getCurrentTimeMs();

            for(auto& task : task_list){
                /* 如果有正在等待运行的任务，则返回0直接进入下一个调度循环。
                 * 一般是因调用了Restart */
                if(task->status == TaskState::WaitingRun){
//                    std::cout << "task->status == TaskState::WaitingRun, min_time_out = 0;"
//                    << std::endl;
                    min_time_out = 0;
                    break;
                }

                /* 如果没有需要立即重启的任务，
                 * 则在正在等待通知且设置了超时的任务中，搜索下一个最小超时*/
                if(task->status == TaskState::WaitingNotify
                   && task->timeout_time_ms != 0) {

                    /* 因时间戳为无符号64位，为避免溢出需要避免负数的出现。
                     * 出现负数的原因是超时的时间点已过。*/
                    if(current_time_ms > task->timeout_time_ms){
                        /* 超时时间点已过
                         * 返回0直接进入下一个调度循环。下面的checkTimeout任务会处理
                         * 已经超时的任务*/
//                        std::cout << "task->timeout_time_ms >= current_time_ms), min_time_out = 0;"
//                                  << std::endl;
                        min_time_out = 0;
                        break;
                    }
                    else{
                        /* 超时时间点尚未到达 */
                        uint64_t sleep_time = task->timeout_time_ms -
                                current_time_ms;
//                        std::cout << "task->timeout_time_ms < "
//                                     "current_time_ms), sleep_time = "
//                                     <<sleep_time<< std::endl;
                        /* 进行限幅，避免下一步强制转换时溢出 */
                        if(sleep_time > MAX_SLEEP_GAP_MS){
                            sleep_time = MAX_SLEEP_GAP_MS;
                        }
                        if(min_time_out > sleep_time){
                            min_time_out = (int)sleep_time;
                        }
                    }
                }
            }

            return min_time_out;
        }

        void checkNotify(){
            if(is_notify_pending){
                for (auto& task : task_list) {
                    if (task->matchNotifyMsg(pending_notify)
                        && task->status == TaskState::WaitingNotify) {

                        task->setStatus(TaskState::WaitingDelete);
                        task->evNotifyCallback(pending_notify);
                    }
                    /* 匹配成功一个任务后即停止，从队列中取出下一个任务后再继续。
                     * 这是因为可能会对同一个地址+消息ID发起多次请求任务
                     * TODO: 改为可配置的（notify_all）*/
                    if(!notify_all){
                        break;
                    }
                }

                is_notify_pending = false;
                notify_ctrl_cv.notify_all();
            }
        }

        void checkRun(){
            for(auto& task : task_list) {
                if(task->status == TaskState::WaitingRun){
                    /*set task status to delete at first,
                     *if restart() or waitNofify() is called,
                     * task->status will be changed. if neither is
                     * called, task will be deleted*/
                    task->setStatus(TaskState::WaitingDelete);
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
                if(task->status != TaskState::WaitingNotify){
//                    std::cout << "task->status != TaskState::WaitingNofity" << std::endl;
                    continue;
                }

                if(task->timeout_time_ms != 0 &&
                    getCurrentTimeMs() >= task->timeout_time_ms){

//                    std::cout << "checkTimeout: timeout task here!" <<
//                    std::endl;

                    /*same reason as above. set to delete first.*/
                    task->setStatus(TaskState::WaitingDelete);
                    task->evTimeoutCallback();
                }
            }
        }

        bool is_start{true};
        bool is_nested_in_evloop{false};

        static bool isWaitingDelete(std::unique_ptr<Task>& task){
            bool matched = task->status == TaskState::WaitingDelete;
//            if(matched){
//                std::cout << "isWaitingDelete!" << std::endl;
//            }
            return matched;
        }

        uint64_t getCurrentTimeMs(){
            if(ms_timesource == nullptr){
                return 0;
            }
            return (*ms_timesource)();
        }

        uint64_t (*ms_timesource)() {nullptr};
        bool notify_all{false};
#ifdef EVENTLOOP_THREADING
        static const int MAX_SLEEP_GAP_MS = 5000;
        std::condition_variable sched_ctrl_cv;
        std::condition_variable notify_ctrl_cv;

        std::mutex update_mutex;
        std::thread* worker_thread{nullptr};
#endif
    };

}

#endif //LIBFCN_EVENTLOOPBASE_HPP
