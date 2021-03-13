//
// Created by sdong on 2020/10/31.
//

#ifndef can_duck_NODE_HPP
#define can_duck_NODE_HPP

#include "Common.hpp"
#include "Message.hpp"
#include "Service.hpp"

#ifdef SYSTYPE_FULL_OS
    #include <vector>
    #include <thread>
    #include <mutex>
#endif //SYSTYPE_FULL_OS

namespace can_duck{

    class Context {
    public:
        Context(LLCanBus* can):
                can(can),
                msg_ctx(can),
                srv_ctx(can){
#ifdef SYSTYPE_FULL_OS
            thrd_recv = new std::thread([&](){
                while (!stop_flag){
                    recvPolling();
                }
                LOGI("Context exit..");
            });
#endif //SYSTYPE_FULL_OS
        }

        ~Context(){
#ifdef SYSTYPE_FULL_OS
            stop();
            spin();
            delete thrd_recv;
#endif //SYSTYPE_FULL_OS
        }

        inline MessageContext& msg(){ return msg_ctx; }
        inline ServiceContext& srv(){ return srv_ctx; }

        void __poll();


    protected:
        LLCanBus* const can;
        MessageContext msg_ctx;
        ServiceContext srv_ctx;

#ifdef SYSTYPE_FULL_OS
        std::thread* thrd_recv{nullptr};
        bool stop_flag{false};

#endif //SYSTYPE_FULL_OS

        void recvPolling();

#ifdef SYSTYPE_FULL_OS
        void stop();
        void spin();
#endif //SYSTYPE_FULL_OS

    };

}

#endif //can_duck_NODE_HPP
