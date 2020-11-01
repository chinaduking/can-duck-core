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
                                                 &cached_request_frame);
    evWaitNotify(timeout_ms);
}


bool RequestTask::matchNotifyMsg(DataLinkFrame& frame){
    return frame.src_id == cached_request_frame.dest_id
           && frame.op_code == ack_op_code
           && frame.msg_id == cached_request_frame.msg_id;
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
}

void RequestTask::onRecv(DataLinkFrame &frame) {

}
