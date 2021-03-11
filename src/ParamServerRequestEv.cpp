//
// Created by sdong on 2019/11/10.
//

#include "ParamServerRequestEv.hpp"
#include "ParamServer.hpp"
#include "NetworkLayer.hpp"

using namespace can_duck;
using namespace emlib;

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
//    return emlib::DefaultAllocator::allocate(size);
    return req_task_obj_pool.allocate();
}

void ParamServerRequestEv::operator delete(void *p) noexcept {
//    emlib::DefaultAllocator::deallocate(p);
    req_task_obj_pool.deallocate(p);
}

void ParamServerRequestEv::evUpdate(){
    context_client->ctx_network_layer->sendFrame(context_client->port_id,
                                                 &cached_req);
    evWaitNotify(timeout_ms);
}


bool ParamServerRequestEv::matchNotifyMsg(FcnFrame& frame){
    return frame.src_id == cached_req.dest_id
           && frame.op_code == ack_op_code
           && frame.msg_id == cached_req.msg_id;
}


void ParamServerRequestEv::evNotifyCallback(FcnFrame& frame){
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

void ParamServerRequestEv::onTimeout() {
    LOGW("request timeout: server = 0x%X, msg_id=0x%X",
         cached_req.dest_id, cached_req.msg_id);
    callback.call(context_client, 2);
}

void ParamServerRequestEv::onRecv(FcnFrame &frame) {

    if(frame.op_code != ack_op_code){
        LOGE("frame.op_code != ack_op_code, %X  & %X, check evloop!",
             frame.op_code, ack_op_code);
        return;
    }

    //TODO: define ev code!
    if(frame.msg_id != cached_req.msg_id){
        LOGE("frame.msg_id != cached_req.msg_id, %X  & %X, check evloop!",
             frame.msg_id, cached_req.msg_id);
        return;
    }


    if(frame.op_code == (uint8_t)OpCode::ParamServer_ReadAck){
        if(context_client->updateData(frame.msg_id, frame.payload)){
            callback.call(context_client, 1);
        } else{
            callback.call(context_client, 3);
        }
    }

    if(frame.op_code == (uint8_t)OpCode::ParamServer_WriteAck){
        callback.call(context_client, frame.payload[0]);
    }
}
