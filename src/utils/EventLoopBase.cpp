//
// Created by sdong on 2019/11/11.
//

#include "EventLoopBase.hpp"
#include "utils/CppUtils.hpp"
#include <cstdlib>
using namespace emros;

#ifdef EVENTLOOP_THREADING
#include <algorithm>
#include <iostream>
using namespace std;
#endif //EVENTLOOP_THREADING

EventBase::EventBase(){
    timeout_time_us = 0;
    status = EVSTAT_WaitingRun;
    context_evloop = nullptr;
}

EventBase::~EventBase(){}

bool EventBase::matchNotifyMsg(DataLinkFrame& msg) {
    return false;
}

void EventBase::evWaitNotify(int timeout_ms){
    /* 传入非正整数使超时时间为无穷 */
    if(timeout_ms <= 0){
        timeout_time_us = 0;
    }else{
        timeout_time_us = getCurrentTimeUs() + timeout_ms * 1000;
    }

    setStatus(EVSTAT_WaitingNofity);
}

void EventBase::evRestart(){
    /*clear timeout to clear waiting signal state.*/

    this->timeout_time_us = 0;

    setStatus(EVSTAT_WaitingRun);
}

void EventBase::evExit(){
    setStatus(EVSTAT_WaitingDelete);
}


void EventBase::setStatus(EventStatus status){
    this->status = status;
}

/*--------------------------------
 * Event Loop Main Scheduler
 *--------------------------------*/

#ifdef EVENTLOOP_THREADING
    #define EVENTLOOP_LOCKGURAD(m_) lock_guard<mutex> evloop_lock_guard(m_)
#else
    #define EVENTLOOP_LOCKGURAD(m_)
#endif

EventLoopBase::EventLoopBase()
{
    is_nested_in_evloop = false;

#ifdef EVENTLOOP_THREADING
    worker_thread = new thread([&](){
        while(scheduleWorker());
    });
#endif

    is_start = true;
}


void EventLoopBase::addTask(ESharedPtr<EventBase>& task)
{
#ifdef EVENTLOOP_THREADING
    unique_lock<mutex> updating_lk(update_mutex);
#endif //EVENTLOOP_THREADING

    task->context_evloop = this;

    if(is_nested_in_evloop){
        task_pool.push(task);
        return;
    }

    task_pool.push(task);

#ifdef EVENTLOOP_THREADING
    sched_ctrl_cv.notify_all();
#endif //EVENTLOOP_THREADING
}


void EventLoopBase::notify()
{
#ifdef EVENTLOOP_THREADING
    unique_lock<mutex> updating_lk(update_mutex);
    sched_ctrl_cv.notify_all();
#endif //EVENTLOOP_THREADING
}


void EventLoopBase::notify(ESharedPtr<DataLinkFrame>& message){
    notify_queue.push(message);
    EventLoopBase::notify();
}


void EventLoopBase::checkNotify() {
    while(!notify_queue.empty()) {
        auto message = notify_queue.head();

        for (auto task : task_pool) {
            if (task->matchNotifyMsg(*message)
            && task->status == EVSTAT_WaitingNofity) {
                task->setStatus(EVSTAT_WaitingDelete);
                task->evNotifyCallback(*message);
            }
        }

        notify_queue.pop();
    }
}

bool EventLoopBase::scheduleWorker(){
#ifdef EVENTLOOP_THREADING
    unique_lock<mutex> updating_lk(update_mutex);
#endif
    /*is_nested_in_evloop flag is use for adding task inside scheduleWorker().
     * without it, adding task will cause deadlock!*/
    is_nested_in_evloop = true;

    /*1. check run*/
    checkRun();

    /* cv is notified.
     * there may exsit two reasons:
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
                updating_lk, chrono::microseconds(min_time_out));
        is_nested_in_evloop = true;
    }
#endif
    is_nested_in_evloop = false;

    return is_start;
}


int EventLoopBase::getMinSleepTime(){

    /* 最大等待时间。当没有任何唤醒/超时事件时，事件循环每0.5s
     * 唤醒自身一次，检查所有任务状态。*/
    /* max wait time, that means if all task timeout is longer than 0.5s,
     * or task pool is empty, the scheduler will weak up every 0.5s.
     */
    int min_time_out = MAX_SLEEP_GAP;

    auto current_time_us = getCurrentTimeUs();
    for(auto task : task_pool){
        /* 如果有正在等待运行的任务，则返回0直接进入下一个循环。
         * 一般是因调用了Restart */
        if(task->status == EVSTAT_WaitingRun){
            min_time_out = 0;
            break;
        }

        /* 如果没有需要立即重启的任务，
         * 则在正在等待通知且设置了超时的任务中，搜索下一个最小超时*/
        if(task->status == EVSTAT_WaitingNofity
         && task->timeout_time_us != 0) {

            uint64_t sleep_time = task->timeout_time_us - current_time_us;

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

void EventLoopBase::checkRun()
{
    for(auto task : task_pool) {
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

void EventLoopBase::checkTimeout() {
    for(auto task : task_pool){
        if(task->status != EVSTAT_WaitingNofity){
            continue;
        }

        if(task->timeout_time_us != 0 && getCurrentTimeUs() > task->timeout_time_us){
            /*same reason as above. set to delete first.*/
            task->setStatus(EVSTAT_WaitingDelete);
            task->evTimeoutCallback();
        }
    }
}


bool isWaitingDelete(EventBase& ev){
    return ev.status == EVSTAT_WaitingDelete;
}

void EventLoopBase::checkDelete()
{
    task_pool.removeIf(isWaitingDelete);
}

void EventLoopBase::spin()
{
    worker_thread->join();
}

void EventLoopBase::stop()
{
    EVENTLOOP_LOCKGURAD(update_mutex);

    is_start = false;
}

EventLoopBase::~EventLoopBase()
{
    stop();
    /* wait until worker thread stop. force delete
     * worker_thread will cause core dump*/
    spin();

#ifdef EVENTLOOP_THREADING
    delete worker_thread;
#endif

    cout << "EventLoopBase deleted.\n";
}
