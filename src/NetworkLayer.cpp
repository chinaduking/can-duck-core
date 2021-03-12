//
// Created by sdong on 2020/10/21.
//

#include "NetworkLayer.hpp"
#include "OpCode.hpp"
#include "Tracer.hpp"

using namespace can_duck;
using namespace emlib;

//int NetworkLayer::addDataLinkDevice(LLCanBus *device) {
//
//    can_bus = device;
//    return 0;
//}

int NetworkLayer::sendFrame(uint16_t port_id, CANMessage *frame) {

//    LOGI("NetworkLayer::sendFrame:\n\r%s",
//         frame2stdstr(*frame).c_str());
//
//    if(port_id >= can_bus.size()){
//        LOGE("NetworkLayer::sendFrame: invalid port:%d", port_id);
//        return -1;
//    }
    if(can_bus == nullptr){
        return 0;
    }

    return can_bus->write(*frame);
}

void NetworkLayer::recvPolling() {
    CANMessage frame_tmp;

    if(can_bus == nullptr){
        return;
    }
//    perciseSleep(1);

    if(!can_bus->read(frame_tmp)){
        return;
    }

    if(pub_sub_manager.handleRecv(&frame_tmp, 0)){
        return;
    }

    if(param_server_manager.handleRecv(&frame_tmp, 0)){
        return;
    }
}

void NetworkLayer::sendPolling() {
    /* 轮询读发送数据
     * 注意，如果有多个端口，且读为阻塞模式，则会多个端口
     * 之间会互相阻塞，因此需要多线程。但一般上位机只有一个端口（串口）
     * 因此先不做优化。下位机一般不采用OS，为非阻塞读取，不受影响。
     * */
//    for(auto& dev  can_bus){
//        dev->sendPolling();
//    }
}