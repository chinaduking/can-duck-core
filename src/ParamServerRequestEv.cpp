//
// Created by sdong on 2019/11/10.
//

#include "ParamServerRequestEv.hpp"
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

ObjPool<ParamServerRequestEv, CLIENT_MAX_REQ_NUM * 2> req_task_obj_pool;

void * ParamServerRequestEv::operator new(size_t size) noexcept {
    //TODO: Request Allocator !
//    return utils::DefaultAllocator::allocate(size);
    return req_task_obj_pool.allocate();
}

void ParamServerRequestEv::operator delete(void *p) noexcept {
//    utils::DefaultAllocator::deallocate(p);
    req_task_obj_pool.deallocate(p);
}

void ParamServerRequestEv::evUpdate(){
    context_client->ctx_network_layer->sendFrame(context_client->port_id,
                                                 &cached_req);
    evWaitNotify(timeout_ms);
}


bool ParamServerRequestEv::matchNotifyMsg(DataLinkFrame& frame){
    return frame.src_id == cached_req.dest_id
           && frame.op_code == ack_op_code
           && frame.msg_id == cached_req.msg_id;
}


void ParamServerRequestEv::evNotifyCallback(DataLinkFrame& frame){
    onRecv(frame);
    evExit();
}


void ParamServerRequestEv::evTimeoutCallback() {
    if(retry_cnt < retry_max){
        retry_cnt ++;
        evRestart();

        LOGD("request timeout retry:: server = 0x%X, msg_id=0x%X (%d/%d)",
             cached_req.dest_id, cached_req.msg_id, retry_cnt, retry_max );

        return;
    }

    onTimeout();
    evExit();
}


void RequestCallback::call(int ev_code, DataLinkFrame *frame){
    if(cb != nullptr){
        (*cb)(ctx_obj, ev_code ,frame);
    }
}


void ParamServerRequestEv::onTimeout() {
    LOGW("request timeout: server = 0x%X, msg_id=0x%X",
         cached_req.dest_id, cached_req.msg_id);
    callback.call(2, nullptr);
}

void ParamServerRequestEv::onRecv(DataLinkFrame &frame) {
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


    if(frame.op_code == (uint8_t)OpCode::ParamServer_ReadAck){
        callback.call(1, nullptr);
    }

    if(frame.op_code == (uint8_t)OpCode::ParamServer_WriteAck){
        callback.call(1, &frame);
    }
}
