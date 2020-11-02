//
// Created by sdong on 2019/11/10.
//

#include "RequestTask.hpp"
#include "ParamServer.hpp"
#include "NetworkLayer.hpp"

using namespace libfcn_v2;
using namespace utils;

ObjPool<LinkedList<std::unique_ptr<int>>,
        CLIENT_MAX_REQ_NUM*2> unique_ptr_node_objpool;

void * LinkedListNodeAllocator::allocate(size_t size) {
    return unique_ptr_node_objpool.allocate();
}

void LinkedListNodeAllocator::deallocate(void *p) {
    unique_ptr_node_objpool.deallocate(p);
}

ObjPool<RequestTask, CLIENT_MAX_REQ_NUM*2> req_task_obj_pool;

void * RequestTask::operator new(size_t size) noexcept {
    //TODO: Request Allocator !
//    return utils::DefaultAllocator::allocate(size);
    return req_task_obj_pool.allocate();
}

void RequestTask::operator delete(void *p) noexcept {
//    utils::DefaultAllocator::deallocate(p);
    req_task_obj_pool.deallocate(p);
}

void RequestTask::evUpdate(){
    context_client->ctx_network_layer->sendFrame(context_client->port_id,
                                                 &cached_req);
    evWaitNotify(timeout_ms);
}


bool RequestTask::matchNotifyMsg(DataLinkFrame& frame){
    return frame.src_id == cached_req.dest_id
           && frame.op_code == ack_op_code
           && frame.msg_id == cached_req.msg_id;
}


void RequestTask::evNotifyCallback(DataLinkFrame& frame){
    onRecv(frame);
    evExit();
}


void RequestTask::evTimeoutCallback() {
    if(retry_cnt < retry_max){
        retry_cnt ++;
        evRestart();
        return;
    }

    onTimeout();
    evExit();
}


void RequestTask::onTimeout() {
    LOGW("request timeout: server = 0x%X, msg_id=0x%X",
         cached_req.dest_id, cached_req.msg_id);
    if(callback.callback_func != nullptr){
        (*callback.callback_func)(callback.obj_ptr, 5,
                                  nullptr);
    }
}

void RequestTask::onRecv(DataLinkFrame &frame) {
//    if(context_client->serdes_dict == nullptr){
//        return;
//    }
    if(frame.op_code != ack_op_code){
        LOGE("frame.op_code != ack_op_code, %X  & %X, check evloop!",
             frame.op_code, ack_op_code);
        return;
    }

    if(frame.msg_id != cached_req.msg_id){
        LOGE("frame.msg_id != cached_req.msg_id, %X  & %X, check evloop!",
             frame.msg_id, cached_req.msg_id);
        return;
    }


    if(frame.op_code == (uint8_t)OpCode::SVO_SINGLE_READ_ACK){
        if(callback.callback_func != nullptr){
            (*callback.callback_func)(callback.obj_ptr, 0, frame.payload);
        }
//        return; //TODO??
    }
    if(frame.op_code == (uint8_t)OpCode::SVO_SINGLE_WRITE_ACK){

        if(callback.callback_func != nullptr){
            (*callback.callback_func)(callback.obj_ptr, frame.payload[0],
                    nullptr);
        }
    }
}
