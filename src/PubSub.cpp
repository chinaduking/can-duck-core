//
// Created by sdong on 2020/10/15.
//

#include "PubSub.hpp"
#include "NetworkLayer.hpp"
#include "OpCode.hpp"

#include "CppUtils.hpp"
#include "Tracer.hpp"

using namespace libfcn_v2;
using namespace emlib;

/*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
obj_size_t libfcn_v2::RtoDictSingleWrite(SerDesDict* obj_dict,
                                         void* buffer,
                                         obj_idx_t index,
                                         uint8_t *data, obj_size_t len){

//    /* 不一次直接memcpy，有两个原因：
//     * 1. 每次均检查index是否已溢出
//     * 2. 支持未来的回调
//     * */
//    while (len > 0){

        USER_ASSERT(buffer != nullptr);

        if(index > obj_dict->dictSize()){
            /* 仅做写保护，不使程序assert failed崩溃：
             * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
             * 可记录错误log
             * */
            return 0;
        }

//        auto p_obj = dict->obj_dict[index];

        auto offset = obj_dict->getBufferDataOffest(index);
        auto data_sz = obj_dict->getBufferDataSize(index);

        USER_ASSERT(data_sz != 0);


        emlib::memcpy((uint8_t*)buffer+offset, data, data_sz);

//        data += p_obj->data_size;
//
//        len -= p_obj->data_size;

//        index ++;
//    }

    return data_sz;
}

void libfcn_v2::singleWriteFrameBuilder(
        FcnFrame* result_frame,
        uint16_t src_id,
        uint16_t dest_id,
        uint16_t op_code,
        uint16_t msg_id,
        uint8_t* p_data, uint16_t len){

    USER_ASSERT(result_frame != nullptr);


    /* 初始化 */
//    result_frame->op_code = ;
    result_frame->src_id  = src_id;
    result_frame->dest_id = dest_id;
    result_frame->op_code = op_code;
    result_frame->msg_id  = msg_id; /* 消息ID为起始ID */

    result_frame->setPayloadLen(len);      /* 开始对数据长度进行累加 */

    uint8_t * payload_ptr = result_frame->payload;


    /* 填充数据 */
    emlib::memcpy(payload_ptr, p_data, len);

}

//void PubSubChannel::networkPublish(FcnFrame *frame) {
//    if(network_layer != nullptr){
//        //TODO: by publish ctrl rules!!
//        network_layer->sendFrame(0, frame);
//    }
//}


/* ---------------------------------------------------------
 *            Realtime Object Transfer Controller
 * ---------------------------------------------------------
 */


void PubSubManager::handleWrtie(FcnFrame* frame, uint16_t recv_port_id) {
    Subscriber* subscriber = nullptr;

    for(auto& sub : created_subscribers){
        if(sub->channel_addr == frame->src_id){
            subscriber = sub;
        }
    }

    /* 未找到对应地址的信道不代表运行错误，一般是因为数据包先到达，但本地字典尚未注册 */
    if(subscriber == nullptr){
        LOGW("PubSubManager::handleWrtie, channel == nullptr\n");

        return;
    }

    auto opcode = static_cast<OpCode>(frame->op_code);

    switch (opcode) {
        case OpCode::Publish:
            RtoDictSingleWrite(
                    subscriber->serdes_dict,
                    subscriber->buffer,
                    frame->msg_id,
                    frame->payload, frame->getPayloadLen());

            break;

        case OpCode::PublishReq:
            break;

        default:
            break;
    }

}

void * PubSubManager::getSharedBuffer(SerDesDict &serdes_dict, int id) {
    void* buffer = nullptr;

    for(auto & sh_b : shared_buffers){
        if(sh_b.id == id){
            buffer = sh_b.buffer;
            USER_ASSERT(buffer != nullptr);
        }
    }
    if(buffer == nullptr){
        buffer = serdes_dict.createBuffer();

        SharedBuffer sh_b = {
                .id = id,
                .buffer = buffer
        };

        shared_buffers.push(sh_b);
    }

    return buffer;

}

Publisher* PubSubManager::bindPublisherToChannel(SerDesDict& serdes_dict,
                                                 uint16_t channel_addr,
                                                 uint16_t node_id){
    auto buffer = getSharedBuffer(serdes_dict, channel_addr);

    for(auto& pub : created_publishers){
        /* 同一通道中，不能有多个相同的发布者（重复创建发布者） */
        USER_IASSERT(
            !((pub->src_id == node_id) && (pub->channel_id == channel_addr)),
            "duplicate publisher!");
    }


    auto publisher = new Publisher(serdes_dict, buffer,
                                   channel_addr, node_id, this);

    created_publishers.push(publisher);

    for(auto& sub : created_subscribers){
        if(sub->channel_addr == publisher->channel_id){
            publisher->regLocalSubscriber(sub);
        }
    }

    return publisher;
}


Publisher* PubSubManager::makeMasterPublisher(SerDesDict& serdes_dict,
                                              uint16_t node_id){
    uint16_t channel_addr = node_id;
    return bindPublisherToChannel(serdes_dict, channel_addr, node_id);
}

Publisher* PubSubManager::makeSlavePublisher(SerDesDict& serdes_dict,
                              uint16_t master_id, uint16_t node_id){
    uint16_t channel_addr = master_id;
    return bindPublisherToChannel(serdes_dict, channel_addr, node_id);
}

Subscriber * PubSubManager::makeSubscriber(SerDesDict &serdes_dict,
                                           uint16_t channel_addr, uint16_t node_id) {
    auto buffer = getSharedBuffer(serdes_dict, channel_addr);

    for(auto& sub : created_subscribers){
        /* 同一通道中，不能有多个相同的发布者（重复创建订阅者） */
        USER_IASSERT(
                !((sub->src_id == node_id) && (sub->channel_addr == channel_addr)),
                "duplicate subscriber!");
    }

    auto subscriber = new Subscriber(serdes_dict, buffer,
                                   channel_addr, node_id, this);

    created_subscribers.push(subscriber);

    for(auto& pub: created_publishers){
        if(pub->channel_id == subscriber->channel_addr){
            pub->regLocalSubscriber(subscriber);
        }
    }

    return subscriber;
}

Publisher & Publisher::addPort(int port) {
    network_pub_ports.push(port);
    return *this;
}

void Publisher::publish(SerDesDictValHandle &msg, bool local_only) {
    USER_ASSERT(ps_manager != nullptr);

    /* 先进行本地发布，即直接将数据拷贝到共享内存中 */
    serdes_dict->handleSerialize(msg, buffer);

    /* 进行本地发布的回调*/
    for(auto& sub: local_sub_ptr){
        sub->notify(msg.index);
    }

    /* 再进行网络发布：*/
    if(network_pub_ports.size() == 0 || local_only){
        return;
    }

    singleWriteFrameBuilder(
            &trans_frame_tmp,
            src_id,
            channel_id,
            static_cast<uint8_t>(OpCode::Publish),
            msg.index,
            (uint8_t *)msg.getDataPtr(), msg.data_size);

    for(auto port : network_pub_ports){
        ps_manager->network_layer->sendFrame(port, &trans_frame_tmp);
    }
}

void Publisher::regLocalSubscriber(Subscriber *subscriber) {
    /* 同一个NodeID的订阅者只能订阅同一个发布者一次。 */
    for(auto& sub : local_sub_ptr){
        if(sub->src_id == subscriber->src_id){
            return;
        }
    }


    local_sub_ptr.push(subscriber);
}