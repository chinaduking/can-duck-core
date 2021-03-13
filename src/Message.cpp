//
// Created by sdong on 2020/10/15.
//

#include "Message.hpp"
#include "OpCode.hpp"
#include "CppUtils.hpp"
#include "Tracer.hpp"

using namespace can_duck;
using namespace emlib;

/*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
obj_size_t can_duck::MsgDictSingleWrite(SerDesDict* obj_dict,
                                        void* buffer,
                                        obj_idx_t index,
                                        uint8_t *data, obj_size_t len){
    USER_ASSERT(buffer != nullptr);

    if(index > obj_dict->dictSize()){
        /* 仅做写保护，不使程序assert failed崩溃：
         * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
         * 可记录错误log
         * */
        return 0;
    }

    auto offset = obj_dict->getBufferDataOffest(index);
    auto data_sz = obj_dict->getBufferDataSize(index);

    USER_ASSERT(data_sz != 0);


    emlib::memcpy((uint8_t*)buffer+offset, data, data_sz);

    return data_sz;
}

void can_duck::fastMessageBuilder(
        CANMessage* result_frame,
        uint16_t node_id,
        uint8_t  dir,
        uint16_t msg_id,
        uint8_t* p_data, uint16_t len){

    USER_ASSERT(result_frame != nullptr);

    /* 初始化 */
    USER_IASSERT(node_id <= 63, "invalid node id!");
    USER_IASSERT(dir <= 1,      "invalid dir!");
    USER_IASSERT(msg_id <= 7,   "only support 3 bit of msg id now!");
    USER_IASSERT(len > 0,       "empty message!");

    //TODO: fast-std-msg

    result_frame->format = CANExtended;

    HeaderFastMsgExt id;
    id.is_seg = 0;
    id.is_msg = 0;
    id.node_id = node_id;
    id.is_tx = dir;
    id.msg_id = msg_id;
    id.is_d1_empty = (len == 1 ? 1:0);
    id.data_0 = p_data[0];
    id.data_1 = (len == 1 ? 0 : p_data[1]);

    result_frame->id = *((uint32_t*)&id);

    if(len <= 2){
        result_frame->len = 0;
        return;
    }

    result_frame->len = len - 2;
    /* 填充数据 */
    emlib::memcpy(result_frame->data, p_data+2, len - 2);

}

/* ---------------------------------------------------------
 *            Realtime Object Transfer Controller
 * ---------------------------------------------------------
 */

int MessageContext::__handleRecv(CANMessage* frame, uint16_t recv_port_id) {
    Subscriber* subscriber = nullptr;
    uint8_t buf[66];

    /* ! is msg*/
    if(frame->format == CANExtended &&
        (frame->id & (((uint32_t) 1) << 27))){
        return 0;
    }

    if(frame->format == CANStandard){
        //TODO: handle fast msg std;
        return 0;
    }

    HeaderFastMsgExt header = *(HeaderFastMsgExt*)(&frame->id);

    for(auto& sub : created_subscribers){
        if(sub->node_id == header.node_id
            && (sub->is_owner == !header.is_tx)){
            subscriber = sub;
        }
    }

    /* 未找到对应地址的信道不代表运行错误，一般是因为数据包先到达，但本地字典尚未注册 */
    if(subscriber == nullptr){
        LOGW("MessageContext::handleWrite, channel == nullptr\n");

        return 1;
    }

    if(header.is_d1_empty){
        buf[0] = header.data_0;
        MsgDictSingleWrite(
                subscriber->serdes_dict,
                subscriber->buffer,
                header.msg_id,
                buf, 1);

        return 1;
    }else{
        buf[0] = header.data_0;
        buf[1] = header.data_1;
        emlib::memcpy(buf+2, frame->data, frame->len);
        MsgDictSingleWrite(
                subscriber->serdes_dict,
                subscriber->buffer,
                header.msg_id,
                buf, frame->len + 2);

        return 1;
    }

    return 0;
}


void * MessageContext::getSharedBuffer(SerDesDict &serdes_dict, uint16_t id, uint8_t sub_id) {
    void* buffer = nullptr;

    uint32_t id_comb = (uint32_t)id<<8 | ((uint32_t)sub_id);

    for(auto & sh_b : shared_buffers){
        if(sh_b.id == id_comb){
            buffer = sh_b.buffer;
            USER_IASSERT(buffer != nullptr,
                         "find a matched shared buffer, but buffer is null.");
        }
    }

    if(buffer == nullptr){
        buffer = serdes_dict.createBuffer();

        SharedBuffer sh_b = {
                .id = 0,
                .buffer = buffer
        };

        sh_b.id = id_comb;

        shared_buffers.push(sh_b);
    }

    return buffer;

}

std::pair<Publisher*, Subscriber*> MessageContext::bindChannel(SerDesDict& serdes_dict_tx,
                                                               SerDesDict& serdes_dict_rx,
                                                               uint16_t node_id,
                                                               bool is_owner_node){
    Publisher*  publisher  = nullptr;
    Subscriber* subscriber = nullptr;

    /* 所有者节点，即某一节点发布自身状态、订阅输入指令。
     * 否则，一个节点被称为非所有者，向远程节点订阅状态，并向远程节点发布指令 */
    if(is_owner_node){
        publisher  = bindPublisherToChannel (serdes_dict_tx, node_id, is_owner_node);
        subscriber = bindSubscriberToChannel(serdes_dict_rx, node_id, is_owner_node);
    } else{
        publisher  = bindPublisherToChannel (serdes_dict_rx, node_id, is_owner_node);
        subscriber = bindSubscriberToChannel(serdes_dict_tx, node_id, is_owner_node);
    }

    return std::make_pair(publisher, subscriber);
}



Publisher* MessageContext::bindPublisherToChannel(SerDesDict& serdes_dict,
                                                  uint16_t node_id,
                                                  bool is_owner){
    /* 获取共享内存。*/
    auto buffer = getSharedBuffer(serdes_dict, node_id, (uint8_t)is_owner);

#if 0
    for(auto& pub : created_publishers){
        /* 同一通道中，不能有多个相同的发布者（重复创建发布者） */
        USER_IASSERT(
            !((pub->src_id == node_id) && (pub->node_id == channel_addr)),
            "duplicate publisher!");
    }

#endif

    auto publisher = new Publisher(serdes_dict, buffer, node_id, this);

    /*记录所有者标志，避免同一ID节点自己订阅自己*/
    publisher->is_owner = is_owner;

    /* 将新发布者加入已创建发布者列表中，便于本地订阅者进行订阅 */
    created_publishers.push(publisher);

    /* 将所有已经创建的订阅者注册到新创建的发布者中 */
    for(auto& sub : created_subscribers){
        /* 主节点发布者可连接多个从节点订阅者，从节点发布者可连接只可连接一个主节点订阅者 */
        if(sub->node_id == publisher->node_id
            && is_owner != sub->is_owner) {
            publisher->regLocalSubscriber(sub);
        }
    }

    return publisher;
}

Subscriber* MessageContext::bindSubscriberToChannel(SerDesDict& serdes_dict,
                                                    uint16_t node_id,
                                                    bool is_owner){
    auto buffer = getSharedBuffer(serdes_dict, node_id, (uint8_t)(!is_owner));

#if 0  /*skip check under refactor*/
    for(auto& sub : created_subscribers){
        /* 同一通道中，不能有多个相同的发布者（重复创建订阅者） */
        USER_IASSERT(
                !((sub->src_id == node_id) && (sub->node_id == channel_addr)),
                "duplicate subscriber!");
    }
#endif
    auto subscriber = new Subscriber(serdes_dict, buffer, node_id, this);

    /*记录所有者标志，避免同一ID节点自己订阅自己*/
    subscriber->is_owner = is_owner;

    created_subscribers.push(subscriber);

    /* 将所有已经创建的发布者注册到新创建的订阅者中 */
    for(auto& pub: created_publishers){
        if(pub->node_id == subscriber->node_id
        && is_owner != pub->is_owner){
            pub->regLocalSubscriber(subscriber);
        }
    }

    return subscriber;
}

void Publisher::publish(hDictItem &msg, bool local_only) {
    USER_ASSERT(ps_manager != nullptr);

    /* 先进行本地发布，即直接将数据拷贝到共享内存中 */
    serdes_dict->handleSerialize(msg, buffer);

    /* 进行本地发布的回调*/
    for(auto& sub: local_sub_ptr){
        sub->notify(msg.index);
    }

    /* 如果指定只进行本地发布，则发布已经完成、返回。*/
    if(local_only){
        return;
    }

    can_duck::fastMessageBuilder(
            &trans_frame_tmp,
            node_id,
            is_owner,  /* 如果是所有者节点的发布者，则是tx类型消息；否则是rx类型的消息*/
            msg.index,
            (uint8_t *)msg.getDataPtr(), msg.data_size);

    ps_manager->__sendFrame(trans_frame_tmp);
}

void Publisher::regLocalSubscriber(Subscriber *subscriber) {
    /* 同一个NodeID的订阅者只能订阅同一个发布者一次。 */

#if 0  /*skip check under refactor*/

    for(auto& sub : local_sub_ptr){
        if(sub->src_id == subscriber->src_id){
            return;
        }
    }
#endif


    local_sub_ptr.push(subscriber);
}