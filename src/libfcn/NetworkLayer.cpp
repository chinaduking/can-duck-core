//
// Created by sdong on 2020/10/21.
//

#include "NetworkLayer.hpp"
#include "OpCode.hpp"
#include "Log.hpp"

using namespace libfcn_v2;
using namespace utils;

int NetworkLayer::addDataLinkDevice(FrameIODevice *device) {

    data_link_dev.push_back(device);
    device->local_device_id = data_link_dev.size() - 1;

    return device->local_device_id;
}

int NetworkLayer::sendFrame(uint16_t port_id, DataLinkFrame *frame) {

    LOGI("NetworkLayer::sendFrame:\n\r%s",
         Frame2Log(*frame).c_str());

    if(port_id >= data_link_dev.size()){
        return -1;
    }

    return data_link_dev[port_id]->write(frame);
}

/* 将不同命令字分配到不同协议上
 * */
void NetworkLayer::recvProtocolDispatcher(DataLinkFrame *frame, uint16_t recv_port_id) {

    LOGI("NetworkLayer::recvProtocolDispatcher:\n\r%s",
         Frame2Log(*frame).c_str());

    auto op_code = frame->op_code;

    /* 实时消息 */
    if(op_code >= (uint8_t)OpCode::Publish
        && op_code <= (uint8_t)OpCode::PublishReq){

        rto_network_handler.handleWrtie(frame, recv_port_id);
    }

    /* 服务消息-d */
    if(op_code >= (uint8_t)OpCode::ParamServer_ReadReq
       && op_code <= (uint8_t)OpCode::ParamServer_WriteAck) {

        svo_network_handler.handleRecv(frame, recv_port_id);
    }


    /*
     * 服务消息的多端口转发:
     * 模拟了CAN总线的"总线模式"，任何消息均为全网转发。
     * 收到了发给别人的数据包，直接广播到所有端口（收到该数据包的端口除外，
     * 避免数据包死循环）*/
    if(op_code >= (uint8_t)OpCode::ParamServer_ReadReq
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

    /* 轮询读取数据
     * 注意，如果有多个端口，且读为阻塞模式，则会多个端口
     * 之间会互相阻塞，因此需要多线程。但一般上位机只有一个端口（串口）
     * 因此先不做优化。下位机一般不采用OS，为非阻塞读取，不受影响。
     * */
    for(auto& dev : data_link_dev){
        if(dev->read(&frame_tmp)){
            recvProtocolDispatcher(&frame_tmp, dev->local_device_id);
        }
    }
}