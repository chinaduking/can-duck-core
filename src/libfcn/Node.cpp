//
// Created by sdong on 2020/10/31.
//

#include "Node.hpp"
#include "NetworkLayer.hpp"
#include "Tracer.hpp"

using namespace libfcn_v2;

NetworkLayer* Node::network_layer = nullptr;

Node::Node() {
    if(network_layer == nullptr){
        network_layer = new NetworkLayer();
    }
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

}

void Node::spinOnce(){

}

#ifdef SYSTYPE_FULL_OS

void Node::spin(){

}

void Node::stop(){

}


#endif //SYSTYPE_FULL_OS