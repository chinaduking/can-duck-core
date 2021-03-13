//
// Created by sdong on 2020/10/31.
//

#include "Context.hpp"
#include "Tracer.hpp"

using namespace can_duck;
using namespace emlib;

void Context::recvPolling(){
    if(can == nullptr){
        LOGE("no can device found. recv nothing");
        return;
    }

    CANMessage rx_msg;
    if(can->read(rx_msg) == 0){
        return;
    }

    if(msg_ctx.handleRecv(&rx_msg, 0)){
        return;
    }
    if(srv_ctx.handleRecv(&rx_msg, 0)){
        return;
    }
}

void Context::poll(){
    recvPolling();
}

#ifdef SYSTYPE_FULL_OS

void Context::spin(){
//    send_threads->join();
    thrd_recv->join();
}

void Context::stop(){
    stop_flag = true;
}


#endif //SYSTYPE_FULL_OS
