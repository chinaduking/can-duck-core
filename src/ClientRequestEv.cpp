//
// Created by sdong on 2019/11/10.
//

#include "ClientRequestEv.hpp"
#include "ParamServer.hpp"
#include "DuckDebug.hpp"

using namespace can_duck;
using namespace emlib;


void toCanMsg(ServiceFrame& srv_frame, CANMessage& msg);

ObjPool<LinkedList<std::unique_ptr<int>>,
        CLIENT_MAX_REQ_NUM*2> unique_ptr_node_objpool;

void * UPtrListAllocator::allocate(size_t size) {
    return unique_ptr_node_objpool.allocate();
}

void UPtrListAllocator::deallocate(void *p) {
    unique_ptr_node_objpool.deallocate(p);
}

ObjPool<ClientRequestEv, CLIENT_MAX_REQ_NUM * 2> req_task_obj_pool;

void * ClientRequestEv::operator new(size_t size) noexcept {
    return req_task_obj_pool.allocate();
}

void ClientRequestEv::operator delete(void *p) noexcept {
    req_task_obj_pool.deallocate(p);
}

void ClientRequestEv::evUpdate(){
    CANMessage can_msg;
    toCanMsg(cached_req, can_msg);

    LOGI("ClientRequestEv::evUpdate send a frame:\n %s", can_duck::frame2stdstr(cached_req).c_str());

    context_client->manager->sendFrame(can_msg);
    evWaitNotify(timeout_ms);
}


bool ClientRequestEv::matchNotifyMsg(ServiceFrame& frame){
    return frame.src_id == cached_req.dest_id
           && frame.op_code == ack_op_code
           && frame.srv_id == cached_req.srv_id;
}


void ClientRequestEv::evNotifyCallback(ServiceFrame& frame){
    onRecv(frame);
    evExit();
}


void ClientRequestEv::evTimeoutCallback() {
    if(retry_cnt < retry_max){
        retry_cnt ++;
        evRestart();

        LOGD("request timeout retry:: server = 0x%X, srv_id=0x%X (%d/%d)",
             cached_req.dest_id, cached_req.srv_id, retry_cnt, retry_max );

        return;
    }

    onTimeout();
    evExit();
}

void ClientRequestEv::onTimeout() {
    LOGW("request timeout: server = 0x%X, srv_id=0x%X",
         cached_req.dest_id, cached_req.srv_id);
    callback.call(context_client, 2);
}

void ClientRequestEv::onRecv(ServiceFrame &frame) {

    if(frame.op_code != ack_op_code){
        LOGE("frame.op_code != ack_op_code, %X  & %X, check evloop!",
             frame.op_code, ack_op_code);
        return;
    }

    //TODO: define ev code!
    if(frame.srv_id != cached_req.srv_id){
        LOGE("frame.srv_id != cached_req.srv_id, %X  & %X, check evloop!",
             frame.srv_id, cached_req.srv_id);
        return;
    }


    if(frame.op_code == (uint8_t)OpCode::ParamServer_ReadAck){
        if(context_client->updateData(frame.srv_id, frame.payload)){
            callback.call(context_client, 1);
        } else{
            callback.call(context_client, 3);
        }
    }

    if(frame.op_code == (uint8_t)OpCode::ParamServer_WriteAck){
        callback.call(context_client, frame.payload[0]);
    }
}
