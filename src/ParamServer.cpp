//
// Created by sdong on 2020/10/15.
//

#include "ParamServer.hpp"
#include "OpCode.hpp"
#include "NetworkLayer.hpp"
#include "DuckDebug.hpp"
using namespace can_duck;

void toCanMsg(ServiceFrame& srv_frame, CANMessage& msg){
    HeaderService header;
    header.src_id       = srv_frame.src_id   ;
    header.dest_id      = srv_frame.dest_id ;
    header.op_code      = srv_frame.op_code ;
    header.service_id   = srv_frame.msg_id  ;
    header.is_msg       = 0;
    header.is_seg       = 0;

    msg.id = *(uint32_t*)&header;
    msg.len = srv_frame.payload_len;

    memcpy( msg.data, srv_frame.payload, msg.len);
}

void fromCanMsg(CANMessage& msg, ServiceFrame& srv_frame){
    HeaderService header = *(HeaderService*)(&msg.id);
    USER_ASSERT(header.is_msg == 0);
    USER_ASSERT(header.is_seg == 0);

    srv_frame.src_id  = header.src_id       ;
    srv_frame.dest_id = header.dest_id      ;
    srv_frame.op_code = header.op_code      ;
    srv_frame.msg_id  = header.service_id   ;

    srv_frame.payload_len = msg.len;
    emlib::memcpy(srv_frame.payload, msg.data, msg.len);
}



/*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度
 * 返回1为成功，否则为失败
 * */
obj_size_t ParamServer::onWriteReq(ServiceFrame* frame,
                                   uint16_t port_id){
    LOGI("onWriteReq recv a frame: %s",
         can_duck::frame2stdstr(*frame).c_str());

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
        if(size != frame->payload_len){
            ack_code = 2;
        }

        /* 写权限检查 */
        if(!wr_access_table.has(index)){
            ack_code = 3;
        }

        /* 检查通过，写入 */
        if(ack_code == 1){
            emlib::memcpy((uint8_t*)buffer + offset ,
                          frame->payload, size);

            //TODO: !回调。 注意成功才会回调
            //    auto callback = p_obj->callback;
            //
            //    if(callback != nullptr){
            //        callback->callback(p_obj->getDataPtr(), 0);
            //    }
        }
    }

    ServiceFrame ack_frame;
    ack_frame.payload[0] = ack_code;
    ack_frame.payload_len = 1;

    ack_frame.msg_id  = frame->msg_id;
    ack_frame.op_code = (uint8_t)OpCode::ParamServer_WriteAck;
    ack_frame.src_id  = frame->dest_id;
    ack_frame.dest_id = frame->src_id;

    /* 先在本地（同进程）已创建的客户端中搜索，如果找到则不再在网络中进行发送 */
    if(!manager->handleRecv(&ack_frame, port_id)){
        CANMessage can_msg;
        toCanMsg(ack_frame, can_msg);
        LOGI("onWriteReq send a frame: %s", can_duck::frame2stdstr(ack_frame).c_str());

        manager->sendFrame(can_msg);
    }

    return ack_code;
}


/* 响应读取请求
 * */
obj_size_t ParamServer::onReadReq(ServiceFrame* frame,
                                  uint16_t port_id){

    LOGI("onReadReq recv a frame: %s",
         can_duck::frame2stdstr(*frame).c_str());

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
    ServiceFrame ack_frame;

    emlib::memcpy(ack_frame.payload,
                  (uint8_t*)buffer + offset, size);

    ack_frame.payload_len = size;

    ack_frame.msg_id  = frame->msg_id;
    ack_frame.op_code = (uint8_t)OpCode::ParamServer_ReadAck;
    ack_frame.src_id  = frame->dest_id;
    ack_frame.dest_id = frame->src_id;

    /* 先在本地（同进程）已创建的客户端中搜索，如果找到则不再在网络中进行发送 */
    if(!manager->handleRecv(&ack_frame, port_id)){
        CANMessage can_msg;
        toCanMsg(ack_frame, can_msg);

        LOGI("onReadReq send a frame:\n %s", can_duck::frame2stdstr(ack_frame).c_str());
        manager->sendFrame(can_msg);
    }

    return 0;
}

void ParamServerClient::onReadAck(ServiceFrame* frame){
    LOGI("onReadAck recv a frame:\n %s", can_duck::frame2stdstr(*frame).c_str());
    ev_loop.notify(*frame);
}

void ParamServerClient::onWriteAck(ServiceFrame* frame){
    LOGI("onWriteAck recv a frame:\n %s", can_duck::frame2stdstr(*frame).c_str());
    ev_loop.notify(*frame);
}

int ParamServerClient::networkSendFrame(uint16_t port_id, ServiceFrame *frame) {
    /* 先在本地（同进程）已创建的服务器中搜索，如果找到则不再在网络中进行发送 */
    if(!manager->handleRecv(frame, port_id)){
        CANMessage can_msg;
        toCanMsg(*frame, can_msg);
        manager->sendFrame(can_msg);
    }
    return 0;
}


/* 不同于Pub-Sub，一个地址只允许存在一个服务器实例 */
ParamServer* ParamServerManager::createServer(SerDesDict& prototype, uint16_t address){
    ParamServer* server = nullptr;
    for(auto & srv : created_servers){
        if(srv.address == address){
            server = srv.instance;
            USER_ASSERT(server != nullptr);
        }
    }

    if(server == nullptr){
        server = new ParamServer(this, address,
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
        client = new ParamServerClient(this, server_addr, client_addr,
                                       port_id,
                                       &prototype,
                                       prototype.createBuffer());

        CreatedClient cli = {
                .address = client_addr,
                .instance = client
        };

        created_clients.push_back(cli);
    }

    return client;
}

int ParamServerManager::handleRecv(CANMessage* can_msg, uint16_t recv_port_id) {
    ServiceFrame frame;
    fromCanMsg(*can_msg, frame);

    return handleRecv(&frame, 0);
}

int ParamServerManager::handleRecv(ServiceFrame* frame, uint16_t recv_port_id) {
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

