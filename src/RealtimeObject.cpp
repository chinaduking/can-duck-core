//
// Created by sdong on 2020/10/15.
//

#include "RealtimeObject.hpp"
#include "OperationCode.hpp"
#include "utils/CppUtils.hpp"
using namespace libfcn_v2;

/* ---------------------------------------------------------
 *               Realtime Object Definition
 * ---------------------------------------------------------
 */

ObjectDict::ObjectDict(index_t dict_size) :
        dict_size(dict_size){
    obj_dict = new ObjDictItemBase*[dict_size];
}

ObjectDict::~ObjectDict() {
    delete [] obj_dict;
}

/*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
data_size_t ObjectDict::singleWrite(index_t index, uint8_t *data, data_size_t len){
    while (len > 0){

        /* 不一次直接memcpy，有两个原因：
         * 1. 每次均检查index是否已溢出
         * 2. 支持未来的回调
         * */
        auto p_obj = (RtoDictItemBase*)getObject(index);

        /* 仅做写保护，不使程序assert failed崩溃：
         * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
         * 可记录错误log
         * */
        if(p_obj == nullptr){
            return 0xFFFF;
        }

        utils::memcpy(p_obj->getDataPtr(), data,
                p_obj->data_size);

        auto callback = p_obj->getCallbackPtr();

        if(callback != nullptr){
            callback->callback(p_obj->getDataPtr(), 0);
        }

        /* 接收计数自增 */
//        p_obj->status_code ++;

        writePostAction(index);

        data += p_obj->data_size;

        len -= p_obj->data_size;

        index ++;
    }

    return 0;
}

ObjDictItemBase *ObjectDict::getObject(index_t index) {

    /* 仅做写保护，不使程序assert failed崩溃：
     * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
     * 可记录错误log
     * */
    if(index >= dict_size){
        return nullptr;
    }

    return obj_dict[index];
}

index_t ObjectDict::getDictSize() {
    return dict_size;
}

void libfcn_v2::RtoFrameBuilder(
        DataLinkFrame* result_frame,
        ObjectDict* dict,
        uint16_t index){

    RtoFrameBuilder(result_frame, dict, index, index);
}

void libfcn_v2::RtoFrameBuilder(
        DataLinkFrame* result_frame,
        ObjectDict* dict,
        uint16_t index_start, uint16_t index_end){

    /* 保证起始地址不高于结束地址 */
    USER_ASSERT(index_start <= index_end);

    /* 初始化 */
    result_frame->op_code = static_cast<uint8_t>(OpCode::RTO_PUB);
    result_frame->payload_len = 0;      /* 开始对数据长度进行累加 */
    result_frame->msg_id = index_start; /* 消息ID为起始ID */
    uint8_t * payload_ptr = result_frame->payload;

    /* 填充数据 */
    for(int index = index_start; index <= index_end; index++){
        auto obj = (RtoDictItemBase*)dict->getObject(index);

        USER_ASSERT(obj != nullptr);
        if(obj == nullptr){ return; }

        utils::memcpy(payload_ptr, obj->getDataPtr(), obj->data_size);

        result_frame->payload_len += obj->data_size; /* 对数据长度进行累加 */
        payload_ptr += obj->data_size;  /* 输出指针自增 */
    }
}


/* ---------------------------------------------------------
 *            Realtime Object Transfer Controller
 * ---------------------------------------------------------
 */

RtoShmManager* RtoShmManager::instance = nullptr;

/* RtoShmManager is a single-instance */
RtoShmManager* RtoShmManager::getInstance(){
    if(instance == nullptr){
        instance = new RtoShmManager();
    }

    return instance;
}


ObjectDict* RtoShmManager::getSharedDictByAddr(uint16_t address){
    /* if we can find an exsiting shm, return it */
    for(auto & managed_item : managed_items){
        if(managed_item.address == address){
            return managed_item.p_dict;
        }
    }

    return nullptr;
}

void RtoNetworkHandler::handleWrtie(DataLinkFrame* frame) {
    auto dict = rto_manager->getSharedDictByAddr(frame->src_id);

    /* 未找到对应地址的字典不代表运行错误，一般是因为数据包到达，但本地字典尚未注册 */
    if(dict == nullptr){
        return;
    }

    dict->singleWrite(frame->msg_id,  frame->payload, frame->payload_len);
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

            if(pub_ctrl_rule.end_idx == 0xFFFF){
                /* Single Write */
                RtoFrameBuilder(&frame_tmp, pub_ctrl_rule.dict,
                                pub_ctrl_rule.start_or_single_idx);
            }else{
                /* Continuous Write */
                RtoFrameBuilder(&frame_tmp, pub_ctrl_rule.dict, pub_ctrl_rule
                        .start_or_single_idx, pub_ctrl_rule.end_idx);
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