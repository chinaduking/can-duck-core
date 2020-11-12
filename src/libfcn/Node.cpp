//
// Created by sdong on 2020/10/31.
//

#include "Node.hpp"
#include "NetworkLayer.hpp"
#include "utils/Tracer.hpp"

using namespace libfcn_v2;
using namespace utils;


NetworkLayer* Node::network_layer = nullptr;

Node::Node() {
    getTracer()->setFilter(Tracer::Level::lDebug);

    if(network_layer == nullptr){
        network_layer = new NetworkLayer();
    }
#ifdef SYSTYPE_FULL_OS
    send_threads = new std::thread([&](){
        LOGW("start one send thread..");
        while (!stop_flag){
            sendPolling();
        }
        LOGW("stop one send thread..");
    });

    recv_threads = new std::thread([&](){
        LOGW("start one recv thread..");
        while (!stop_flag){
            recvPolling();
        }
        LOGW("stop one recv thread..");
    });

#endif //SYSTYPE_FULL_OS
}

PublisherManager* Node::getPublisherManager(){
    return &network_layer->pub_sub_manager;
}

ParamServerManager* Node::getParamServerManager(){
    return &network_layer->param_server_manager;
}

int Node::addPort(FrameIODevice *device) {
    return network_layer->addDataLinkDevice(device);
}

void Node::sendPolling(){
    network_layer->sendPolling();
}

void Node::recvPolling(){
    network_layer->recvPolling();
}

void Node::spinOnce(){
    sendPolling();
    recvPolling();
}

#ifdef SYSTYPE_FULL_OS

void Node::spin(){
    send_threads->join();
    recv_threads->join();
}

void Node::stop(){
    stop_flag = true;
}


#endif //SYSTYPE_FULL_OS