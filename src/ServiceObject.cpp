//
// Created by sdong on 2020/10/15.
//

#include "ServiceObject.hpp"
#include "OperationCode.hpp"

#include "NetworkLayer.hpp"

using namespace libfcn_v2;

/*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度
 * 返回0为成功，否则为失败
 * */
obj_size_t ServiceObjectDict::singleWrite(obj_idx_t index, uint8_t *data,
                                          obj_size_t len){

    if(index > obj_dict.size()){
        /* 仅做写保护，不使程序assert failed崩溃：
         * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
         * 可记录错误log
         * */
        return 2;
    }

    auto p_obj = obj_dict[index];

    USER_ASSERT(p_obj != nullptr);

    /* 写权限检查 */
    if(p_obj->wr_access == 0){
        return 1;
    }

    /* 单数据写入，要求长度要求必须匹配 */
    if(len != p_obj->data_size){
        return 3;
    }

    utils::memcpy(p_obj->getDataPtr(), data,
                  p_obj->data_size);

    auto callback = p_obj->callback;

    if(callback != nullptr){
        callback->callback(p_obj->getDataPtr(), 0);
    }

    return 0;
}

SvoServer::SvoServer():
        network(NetworkLayer::getInstance())
        {
}

DataLinkFrame server_frame;

void SvoServer::handleRecv(DataLinkFrame *frame, uint16_t recv_port_id) {
    auto opcode = static_cast<OpCode>(frame->op_code);

    switch (opcode) {

        case OpCode::SVO_SINGLE_READ_REQ: {

            if (frame->msg_id >= dict->obj_dict.size()) {
                return;
            }

            auto obj = dict->obj_dict[frame->msg_id];

            USER_ASSERT(obj != nullptr);

            utils::memcpy(server_frame.payload, obj->getDataPtr(),
                          obj->data_size);
            server_frame.payload_len = obj->data_size;
            server_frame.op_code = (uint8_t)OpCode::SVO_SINGLE_READ_ACK;

            //TODO: rd ack
            network->data_link_dev[recv_port_id]->write(&server_frame);
        }
        break;


        case OpCode::SVO_SINGLE_WRITE_REQ: {
            server_frame.payload[0] =
                    dict->singleWrite(frame->msg_id,
                                      frame->payload,
                                      frame->payload_len);

            server_frame.payload_len = 1;

            server_frame.msg_id = frame->msg_id;
            server_frame.op_code = (uint8_t)OpCode::SVO_SINGLE_WRITE_ACK;
            server_frame.src_id = address;
            server_frame.dest_id = frame->src_id;

            network->data_link_dev[recv_port_id]->write(&server_frame);
        }
        break;


        default:
            break;
    }
}