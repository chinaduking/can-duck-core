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
    device->local_device_id = data_link_dev.size() - 1;

    return device->local_device_id;
}

void NetworkLayer::recvDispatcher(DataLinkFrame *frame, uint16_t recv_port_id) {

    auto op_code = frame->op_code;

    if(op_code >= (uint8_t)OpCode::RTO_PUB
        && op_code <= (uint8_t)OpCode::RTO_REQUEST){

        rto_network_handler.handleWrtie(frame, recv_port_id);
        return;
    }

    if(op_code >= (uint8_t)OpCode::SVO_SINGLE_READ_REQ
       && op_code <= (uint8_t)OpCode::SVO_SINGLE_WRITE_ACK){

        for(auto& server : svo_server_local){

            /* 收到发给自己的数据包，进行处理 */
            if(frame->dest_id == server->address){
                server->handleRecv(frame, recv_port_id);
                break;
            }

            /* 收到了发给别人的数据包，直接广播到所有端口（收到该数据包的端口除外，
             * 避免数据包死循环）*/
            for(auto& port : data_link_dev){
                if(port->local_device_id != recv_port_id){
                    //TODO: shared pointer needed??
                    port->write(frame);
                }
            }

        }

        return;
    }
}

void NetworkLayer::recvPolling() {
    DataLinkFrame frame_tmp;

    for(auto& dev : data_link_dev){
        dev->read(&frame_tmp);
        recvDispatcher(&frame_tmp, dev->local_device_id);
    }
}