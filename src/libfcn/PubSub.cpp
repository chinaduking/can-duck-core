//
// Created by sdong on 2020/10/15.
//

#include "PubSub.hpp"
#include "NetworkLayer.hpp"
#include "OpCode.hpp"

#include "utils/CppUtils.hpp"
#include "utils/Tracer.hpp"

using namespace libfcn_v2;
using namespace utils;

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


        utils::memcpy((uint8_t*)buffer+offset, data, data_sz);

//        auto callback = p_obj->getCallbackPtr();
//
//        if(callback != nullptr){
//            callback->callback(p_obj->getDataPtr(), 0);
//        }
//
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
    utils::memcpy(payload_ptr, p_data, len);

}

void PubSubChannel::networkPublish(FcnFrame *frame) {
    if(network_layer != nullptr){
        //TODO: by publish ctrl rules!!
        network_layer->sendFrame(0, frame);
    }
}


/* ---------------------------------------------------------
 *            Realtime Object Transfer Controller
 * ---------------------------------------------------------
 */


void PublisherManager::handleWrtie(FcnFrame* frame, uint16_t recv_port_id) {
    PubSubChannel* channel = nullptr;

    for(auto& ch : pub_sub_channels){
        if((ch->channel_addr == frame->src_id)
            || (ch->is_multi_source && ch->channel_addr == frame->dest_id)){
            //TODO: is_multi_source && handle dest id!!
            channel = ch;
        }
    }

    /* 未找到对应地址的信道不代表运行错误，一般是因为数据包先到达，但本地字典尚未注册 */
    if(channel == nullptr){
        LOGW("RtoNetworkHandler::handleWrtie,channel == nullptr\n");

        return;
    }

    auto opcode = static_cast<OpCode>(frame->op_code);

    switch (opcode) {
        case OpCode::Publish:
            RtoDictSingleWrite(
                    channel->serdes_dict,
                    channel->buffer,
                    frame->msg_id,
                    frame->payload, frame->getPayloadLen());

            break;

        case OpCode::PublishReq:
            break;

        default:
            break;
    }

}

PubSubChannel* PublisherManager::createChannel(SerDesDict& prototype, uint16_t address){
    void* buffer = nullptr;
    for(auto & sh_b : shared_buffers){
        if(sh_b.id == address){
            buffer = sh_b.buffer;
            USER_ASSERT(buffer != nullptr);
        }
    }
    if(buffer == nullptr){
        buffer = prototype.createBuffer();
        SharedBuffer sh_b = {
                .id = address,
                .buffer = buffer
        };

        shared_buffers.push_back(sh_b);
    }

    auto channel = new PubSubChannel(&prototype, buffer);
    channel->network_layer = ctx_network_layer;
    channel->channel_addr = address;


    pub_sub_channels.push_back(channel);

    return channel;
}

//TODO: impl this!!
PubSubChannel* PublisherManager::createChannel(SerDesDict& prototype, uint16_t address,
                             void* static_buffer){
    auto channel = new PubSubChannel(&prototype, static_buffer);
    channel->network_layer = ctx_network_layer;
    channel->channel_addr = address;
    return channel;
}


void PublisherManager::addPubCtrlRule(PubCtrlRule& rule){
    pub_ctrl_rules.push_back(rule);
}

FcnFrame frame_tmp;

void PublisherManager::update(){

    for(auto & pub_ctrl_rule : pub_ctrl_rules){
        pub_ctrl_rule.freq_divier_cnt ++;
        //TODO: ">=" ??
        if(pub_ctrl_rule.freq_divier_cnt > pub_ctrl_rule.freq_divier){

            //TODO..
            if(pub_ctrl_rule.end_idx == -1){
                /* Single Write */
//                singleWriteFrameBuilder(&frame_tmp, pub_ctrl_rule.dict,
//                                            pub_ctrl_rule.start_or_single_idx,
//                                            pub_ctrl_rule.start_or_single_idx,
//                                            pub_ctrl_rule.src_address,
//                                            pub_ctrl_rule.dest_address,
//                                            static_cast<uint8_t>(OpCode::RTO_PUB));
            }else{
                /* Continuous Write */
//                singleWriteFrameBuilder(&frame_tmp, pub_ctrl_rule.dict,
//                                            pub_ctrl_rule.start_or_single_idx,
//                                            pub_ctrl_rule.end_idx,
//                                            pub_ctrl_rule.src_address,
//                                            pub_ctrl_rule.dest_address,
//                                            static_cast<uint8_t>(OpCode::RTO_PUB));
            }

            frame_tmp.src_id = pub_ctrl_rule.src_address;
            frame_tmp.dest_id = pub_ctrl_rule.dest_address;

            for(auto & port : pub_ctrl_rule.data_link_dev){
                /*
                 * 一般write采用非阻塞模式。
                 * 对于RTO，如果本次写入失败，则直接放弃，但同时记录该信息以便下次调整频率。
                 * */
                bool is_busy = port->pushTxQueue(&frame_tmp);

                if(is_busy){
                    pub_ctrl_rule.send_busy_cnt ++;
                }
            }


        }
    }
}