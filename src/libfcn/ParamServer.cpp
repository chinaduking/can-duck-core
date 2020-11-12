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
obj_size_t ParamServer::onWriteReq(FcnFrame* frame,
                                   uint16_t port_id){

    auto index = frame->msg_id;

    uint8_t ack_code = 1;

    /* 仅做写保护，不使程序assert failed崩溃：
     * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
     * 可记录错误log
     * */
    if(index > serdes_dict->dictSize()){
        ack_code = 4;
    }

    if(ack_code == 1){
        auto offset = serdes_dict->getBufferDataOffest(index);
        auto size   = serdes_dict->getBufferDataSize(index);

        USER_ASSERT(size != 0);

        /* 单数据写入，要求长度要求必须匹配 */
        if(size != frame->getPayloadLen()){
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
    FcnFrame ack_frame;
    ack_frame.payload[0] = ack_code;
    ack_frame.setPayloadLen(1);

    ack_frame.msg_id  = frame->msg_id;
    ack_frame.op_code = (uint8_t)OpCode::ParamServer_WriteAck;
    ack_frame.src_id  = frame->dest_id;
    ack_frame.dest_id = frame->src_id;

    /* 先在本地（同进程）已创建的客户端中搜索，如果找到则不再在网络中进行发送 */
    if(!ctx_network_layer->param_server_manager.handleRecv(&ack_frame, port_id)){
        ctx_network_layer->sendFrame(port_id, &ack_frame);
    }

    return ack_code;
}


/* 响应读取请求
 * */
obj_size_t ParamServer::onReadReq(FcnFrame* frame,
                                  uint16_t port_id){

    auto index = frame->msg_id;

    if(index > serdes_dict->dictSize()){
        /* 仅做写保护，不使程序assert failed崩溃：
         * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
         * 可记录错误log
         * */
        return 2;
    }

    auto offset = serdes_dict->getBufferDataOffest(index);
    auto size   = serdes_dict->getBufferDataSize(index);

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
    FcnFrame ack_frame;

    utils::memcpy(ack_frame.payload,
                  (uint8_t*)buffer + offset, size);

    ack_frame.setPayloadLen(size);

    ack_frame.msg_id  = frame->msg_id;
    ack_frame.op_code = (uint8_t)OpCode::ParamServer_ReadAck;
    ack_frame.src_id  = frame->dest_id;
    ack_frame.dest_id = frame->src_id;

    /* 先在本地（同进程）已创建的客户端中搜索，如果找到则不再在网络中进行发送 */
    if(!ctx_network_layer->param_server_manager.handleRecv(&ack_frame, port_id)){
        ctx_network_layer->sendFrame(port_id, &ack_frame);
    }

    return 0;
}

void ParamServerClient::onReadAck(FcnFrame* frame){
//   TODO:
//    event_loop->notify(frame);
     ev_loop.notify(*frame);
}

void ParamServerClient::onWriteAck(FcnFrame* frame){
//   TODO:
//    event_loop->notify(frame);
    ev_loop.notify(*frame);
}

int ParamServerClient::networkSendFrame(uint16_t port_id, FcnFrame *frame) {
    if(ctx_network_layer == nullptr){
        return -1;
    }

    /* 先在本地（同进程）已创建的服务器中搜索，如果找到则不再在网络中进行发送 */
    if(!ctx_network_layer->param_server_manager.handleRecv(frame, port_id)){
        ctx_network_layer->sendFrame(port_id, frame);
    }

    return 0;
}

/* 不同于Pub-Sub，一个地址只允许存在一个服务器实例 */
ParamServer* ParamServerManager::createServer(SerDesDict& prototype, uint16_t
address){
    ParamServer* server = nullptr;
    for(auto & srv : created_servers){
        if(srv.address == address){
            server = srv.instance;
            USER_ASSERT(server != nullptr);
        }
    }

    if(server == nullptr){
        server = new ParamServer(ctx_network_layer, address,
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


ParamServerClient* ParamServerManager::bindClientToServer(
        SerDesDict& prototype,
        uint16_t server_addr,
          uint16_t client_addr,
          uint16_t port_id){ //TODO: remove port id.
    ParamServerClient* client = nullptr;
    for(auto & cli : created_clients){
        if(cli.address == client_addr){
            client = cli.instance;
            USER_ASSERT(client != nullptr);
        }
    }

    if(client == nullptr){
        client = new ParamServerClient(ctx_network_layer, server_addr, client_addr,
                                       port_id, prototype.createBuffer());

        CreatedClient cli = {
                .address = client_addr,
                .instance = client
        };

        created_clients.push_back(cli);
    }

    return client;
}

int ParamServerManager::handleRecv(FcnFrame *frame, uint16_t recv_port_id) {
    auto opcode = static_cast<OpCode>(frame->op_code);

    int matched = 0;

    switch (opcode) {

        case OpCode::ParamServer_ReadReq: {
            for(auto& server : created_servers){
                if(server.address == frame->dest_id){
                    USER_ASSERT(server.instance != nullptr);
                    server.instance->onReadReq(frame, recv_port_id);
                    matched = 1;
                }
            }
        }
            break;


        case OpCode::ParamServer_WriteReq: {
            for(auto& server : created_servers){
                if(server.address == frame->dest_id){
                    USER_ASSERT(server.instance != nullptr);
                    server.instance->onWriteReq(frame, recv_port_id);
                    matched = 1;
                }
            }
        }
            break;


        case OpCode::ParamServer_ReadAck: {
            for(auto& client : created_clients){
                if(client.address == frame->dest_id){
                    USER_ASSERT(client.instance != nullptr);
                    client.instance->onReadAck(frame);
                    matched = 1;
                }
            }
        }
            break;


        case OpCode::ParamServer_WriteAck: {
            for(auto& client : created_clients){
                if(client.address == frame->dest_id){
                    USER_ASSERT(client.instance != nullptr);
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

