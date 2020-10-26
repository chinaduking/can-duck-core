//
// Created by sdong on 2020/10/21.
//

#include "NetworkLayer.hpp"
#include "OperationCode.hpp"


using namespace libfcn_v2;


NetworkLayer* NetworkLayer::instance = nullptr;

NetworkLayer * NetworkLayer::getInstance() {
    if(instance == nullptr){
        instance = new NetworkLayer();
    }

    return instance;
}

int NetworkLayer::addDataLinkDevice(FrameIODevice *device) {

    data_link_dev.push_back(device);
    return data_link_dev.size() - 1;
}

void NetworkLayer::recvDispatcher(DataLinkFrame *frame) {

    auto op_code = frame->op_code;

    if(op_code >= (uint8_t)OpCode::RTO_PUB
        && op_code <= (uint8_t)OpCode::RTO_REQUEST){

        rto_network_handler.handleWrtie(frame);
        return;
    }

    if(op_code >= (uint8_t)OpCode::SVO_SINGLE_READ_REQ
       && op_code <= (uint8_t)OpCode::SVO_SINGLE_WRITE_ACK){

        svo_server.handleRecv(frame);
        return;
    }
}

void NetworkLayer::recvPolling() {
    DataLinkFrame frame_tmp;

    for(auto& dev : data_link_dev){
        dev->read(&frame_tmp);
        recvDispatcher(&frame_tmp);
    }
}