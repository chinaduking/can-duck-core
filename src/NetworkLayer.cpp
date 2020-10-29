//
// Created by sdong on 2020/10/21.
//

#include "NetworkLayer.hpp"
#include "OperationCode.hpp"
#include "TracerSingleton.hpp"

using namespace libfcn_v2;
using namespace utils;

//NetworkLayer* NetworkLayer::instance = nullptr;
//
//NetworkLayer * NetworkLayer::getInstance() {
//    if(instance == nullptr){
//        instance = new NetworkLayer();
//    }
//
//    return instance;
//}

int NetworkLayer::addDataLinkDevice(FrameIODevice *device) {

    data_link_dev.push_back(device);
    device->local_device_id = data_link_dev.size() - 1;

    return device->local_device_id;
}

void NetworkLayer::recvDispatcher(DataLinkFrame *frame, uint16_t recv_port_id) {
    auto tracer = TracerSingleton::getInstance();

//    tracer->print(Tracer::VERBOSE, "NetworkLayer::recvDispatcher:"
//                                   "\n\r%s", DataLinkFrameToString(*frame)
//                                   .c_str());


    auto op_code = frame->op_code;

    /* 实时消息 */
    if(op_code >= (uint8_t)OpCode::RTO_PUB
        && op_code <= (uint8_t)OpCode::RTO_REQUEST){

        rto_network_handler.handleWrtie(frame, recv_port_id);
        return;
    }

    /* 服务消息 */
    if(op_code >= (uint8_t)OpCode::SVO_SINGLE_READ_REQ
       && op_code <= (uint8_t)OpCode::SVO_SINGLE_WRITE_ACK){

        svo_network_handler.handleRecv(frame, recv_port_id);
//        for(auto& server : svo_server_local){
//            /* 收到发给自己的数据包，进行处理 */
//            if(frame->dest_id == server->address){
//                server->handleRecv(frame, recv_port_id);
//                break;
//            }
//        }
    }

//    /* 服务应答消息 */
//    if(op_code == (uint8_t)OpCode::SVO_SINGLE_READ_ACK
//       || op_code == (uint8_t)OpCode::SVO_SINGLE_WRITE_ACK){
//
//    }


    /*
     * 服务消息的多端口转发:
     * 模拟了CAN总线的"总线模式"，任何消息均为全网转发。
     * 收到了发给别人的数据包，直接广播到所有端口（收到该数据包的端口除外，
     * 避免数据包死循环）*/
    if(op_code >= (uint8_t)OpCode::SVO_SINGLE_READ_REQ
    && op_code <= (uint8_t)OpCode::SVO_MULTI_WRITE_VERIFY_ACK){
        for(auto& port : data_link_dev){
            if(port->local_device_id != recv_port_id){
                //TODO: shared pointer needed??
                port->write(frame);
            }
        }
    }

}

void NetworkLayer::recvPolling() {
    DataLinkFrame frame_tmp;

    for(auto& dev : data_link_dev){
        dev->read(&frame_tmp);
        recvDispatcher(&frame_tmp, dev->local_device_id);
    }
}