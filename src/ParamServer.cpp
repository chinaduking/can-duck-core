//
// Created by sdong on 2020/10/15.
//

#include "ParamServer.hpp"
#include "OpCode.hpp"

#include "NetworkLayer.hpp"

using namespace libfcn_v2;

/*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度
 * 返回1为成功，否则为失败
 * */
obj_size_t SvoServer::onWriteReq(DataLinkFrame* frame,
                                 uint16_t port_id){

    auto index = frame->msg_id;

    uint8_t ack_code = 1;

    /* 仅做写保护，不使程序assert failed崩溃：
     * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
     * 可记录错误log
     * */
    if(index > obj_dict_prototype->dictSize()){
        ack_code = 4;
    }

    if(ack_code == 1){
        auto offset = obj_dict_prototype->getBufferDataOffest(index);
        auto size   = obj_dict_prototype->getBufferDataSize(index);

        USER_ASSERT(size != 0);

        /* 单数据写入，要求长度要求必须匹配 */
        if(size != frame->payload_len){
            ack_code = 2;
        }

        /* 写权限检查 */
        if(!wr_access_table.has(index)){
            ack_code = 3;
        }

        /* 检查通过，写入 */
        if(ack_code == 1){
            utils::memcpy((uint8_t*)buffer + offset ,
                          frame->payload, size);

            //TODO: !回调。 注意成功才会回调
            //    auto callback = p_obj->callback;
            //
            //    if(callback != nullptr){
            //        callback->callback(p_obj->getDataPtr(), 0);
            //    }
        }
    }

    //TODO: zero copy frame?
    // 也许在采用Frame优化过的拷贝方法后，不需要（SVO实时性不强）
    DataLinkFrame ack_frame;
    ack_frame.payload[0] = ack_code;
    ack_frame.payload_len = 1;

    ack_frame.msg_id  = frame->msg_id;
    ack_frame.op_code = (uint8_t)OpCode::SVO_SINGLE_WRITE_ACK;
    ack_frame.src_id  = frame->dest_id;
    ack_frame.dest_id = frame->src_id;

    /* 先在本地（同进程）已创建的客户端中搜索，如果找到则不再在网络中进行发送 */
    if(!network_layer->svo_network_handler.handleRecv(frame, port_id)){
        network_layer->sendFrame(port_id, &ack_frame);
    }

    return ack_code;
}


/* 响应读取请求
 * */
obj_size_t SvoServer::onReadReq(DataLinkFrame* frame,
                                 uint16_t port_id){

    auto index = frame->msg_id;

    if(index > obj_dict_prototype->dictSize()){
        /* 仅做写保护，不使程序assert failed崩溃：
         * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
         * 可记录错误log
         * */
        return 2;
    }

    auto offset = obj_dict_prototype->getBufferDataOffest(index);
    auto size   = obj_dict_prototype->getBufferDataSize(index);

    USER_ASSERT(size != 0);

    /* 单数据读取，要求长度要求必须匹配。长度为数据位第一位 */
    if(size != frame->payload[0]){
        return 3;
    }

    if(size > DATALINK_MTU){
        return 4;
    }

    /* 检查通过，构造返回帧 */


    //TODO: zero copy frame?
    // 也许在采用Frame优化过的拷贝方法后，不需要（SVO实时性不强）
    DataLinkFrame ack_frame;

    utils::memcpy(ack_frame.payload,
                  (uint8_t*)buffer + offset, size);

    ack_frame.payload_len = size;

    ack_frame.msg_id  = frame->msg_id;
    ack_frame.op_code = (uint8_t)OpCode::SVO_SINGLE_READ_ACK;
    ack_frame.src_id  = frame->dest_id;
    ack_frame.dest_id = frame->src_id;

    /* 先在本地（同进程）已创建的客户端中搜索，如果找到则不再在网络中进行发送 */
    if(!network_layer->svo_network_handler.handleRecv(frame, port_id)){
        network_layer->sendFrame(port_id, &ack_frame);
    }

    return 0;
}



void SvoClient::onReadAck(DataLinkFrame* frame){
//   TODO:
//    event_loop->notify(frame);
}

void SvoClient::onWriteAck(DataLinkFrame* frame){
//   TODO:
//    event_loop->notify(frame);
}

int SvoClient::networkSendFrame(uint16_t port_id, DataLinkFrame *frame) {
    if(network_layer == nullptr){
        return -1;
    }

    /* 先在本地（同进程）已创建的服务器中搜索，如果找到则不再在网络中进行发送 */
    if(!network_layer->svo_network_handler.handleRecv(frame, port_id)){
        network_layer->sendFrame(port_id, frame);
    }

    return 0;
}

/* 不同于Pub-Sub，一个地址只允许存在一个服务器实例 */
SvoServer* SvoNetworkHandler::createServer(SerDesDict& prototype, uint16_t
address){
    SvoServer* server = nullptr;
    for(auto & srv : created_servers){
        if(srv.address == address){
            server = srv.instance;
            USER_ASSERT(server != nullptr);
        }
    }

    if(server == nullptr){
        server = new SvoServer(network, address,
                               &prototype,
                               prototype.createBuffer());

        CreatedServer srv = {
                .address = address,
                .instance = server
        };

        created_servers.push_back(srv);
    }

    return server;
}


SvoClient* SvoNetworkHandler::bindClientToServer(uint16_t server_addr,
                                                 uint16_t client_addr,
                                                 uint16_t port_id){
    auto client = new SvoClient(network, server_addr, client_addr,
                                port_id);

    CreatedClient cli = {
            .address = client_addr,
            .instance = client
    };

    created_clients.push_back(cli);

    return client;
}

int SvoNetworkHandler::handleRecv(DataLinkFrame *frame, uint16_t recv_port_id) {
    auto opcode = static_cast<OpCode>(frame->op_code);

    int matched = 0;

    switch (opcode) {

        case OpCode::SVO_SINGLE_READ_REQ: {
            for(auto& server : created_servers){
                if(server.address == frame->dest_id){
                    server.instance->onReadReq(frame, recv_port_id);
                    matched = 1;
                }
            }
        }
            break;


        case OpCode::SVO_SINGLE_WRITE_REQ: {
            for(auto& server : created_servers){
                if(server.address == frame->dest_id){
                    server.instance->onWriteReq(frame, recv_port_id);\
                    matched = 1;
                }
            }
        }
            break;


        case OpCode::SVO_SINGLE_READ_ACK: {
            for(auto& client : created_clients){
                if(client.address == frame->dest_id){
                    client.instance->onReadAck(frame);
                    matched = 1;
                }
            }
        }
            break;


        case OpCode::SVO_SINGLE_WRITE_ACK: {
            for(auto& client : created_clients){
                if(client.address == frame->dest_id){
                    client.instance->onWriteAck(frame);
                    matched = 1;
                }
            }

        }
            break;
        default:
            break;
    }

    return matched;
}

