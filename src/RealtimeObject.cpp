//
// Created by sdong on 2020/10/15.
//

#include "RealtimeObject.hpp"
#include "NetworkLayer.hpp"
#include "OperationCode.hpp"
#include "utils/CppUtils.hpp"

#include "TracerSingleton.hpp"

using namespace libfcn_v2;
using namespace utils;

/*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
obj_size_t libfcn_v2::RtoDictSingleWrite(ObjectDictMM* obj_dict,
                              obj_idx_t index,
                              uint8_t *data, obj_size_t len){

//    /* 不一次直接memcpy，有两个原因：
//     * 1. 每次均检查index是否已溢出
//     * 2. 支持未来的回调
//     * */
//    while (len > 0){

        if(index > obj_dict->dictSize()){
            /* 仅做写保护，不使程序assert failed崩溃：
             * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
             * 可记录错误log
             * */
            return 0;
        }

//        auto p_obj = dict->obj_dict[index];

        auto p_data = obj_dict->getBufferDataPtr(index);

        USER_ASSERT(p_data != nullptr);

        auto data_sz = obj_dict->getBufferDataSize(index);

        USER_ASSERT(data_sz != 0);


        utils::memcpy(p_data, data, data_sz);

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
        DataLinkFrame* result_frame,
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

    result_frame->payload_len = len;      /* 开始对数据长度进行累加 */

    uint8_t * payload_ptr = result_frame->payload;


    /* 填充数据 */
    utils::memcpy(payload_ptr, p_data, len);

}

void PubSubChannel::networkPublish(DataLinkFrame *frame) {
    auto tracer = TracerSingleton::getInstance();

    tracer->print(Tracer::INFO, "PubSubChannel::networkPublish.\n%s",
                  DataLinkFrameToString(*frame).c_str());

    if(network_layer != nullptr){
        network_layer->data_link_dev[0]->write(frame);
    }
}


/* ---------------------------------------------------------
 *            Realtime Object Transfer Controller
 * ---------------------------------------------------------
 */


void RtoNetworkHandler::handleWrtie(DataLinkFrame* frame, uint16_t recv_port_id) {
    auto dict = dict_manager.find(frame->src_id);

    /* 未找到对应地址的字典不代表运行错误，一般是因为数据包先到达，但本地字典尚未注册 */
    if(dict == nullptr){
        return;
    }

    auto opcode = static_cast<OpCode>(frame->op_code);

    switch (opcode) {
        case OpCode::RTO_PUB:
            RtoDictSingleWrite(
                    dict,
                    frame->msg_id,
                    frame->payload, frame->payload_len);

            break;

        case OpCode::RTO_REQUEST:
            break;

        default:
            break;
    }

}

void RtoNetworkHandler::addPubCtrlRule(PubCtrlRule& rule){
    pub_ctrl_rules.push_back(rule);
}

DataLinkFrame frame_tmp;

void RtoNetworkHandler::update(){

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
                bool is_busy = port->write(&frame_tmp);

                if(is_busy){
                    pub_ctrl_rule.send_busy_cnt ++;
                }
            }


        }
    }
}