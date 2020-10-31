//
// Created by sdong on 2019/11/10.
//

#include "RequestTask.hpp"
#include "ParamServer.hpp"
#include "NetworkLayer.hpp"

using namespace libfcn_v2;

void RequestTask::evUpdate(){
    context_client->ctx_network_layer->sendFrame(context_client->port_id,
                                                 &cached_request_frame);
    evWaitNotify(timeout_ms);
}


bool RequestTask::matchNotifyMsg(DataLinkFrame& frame){
    return frame.src_id == server_addr
           && frame.op_code == ack_op_code
           && frame.msg_id == msg_id;
}


void RequestTask::evNotifyCallback(DataLinkFrame& frame){
    handleReceive(frame);
    evExit();
}


void RequestTask::evTimeoutCallback() {
    if(retry_cnt < retry_max){
        retry_cnt ++;
        evRestart();
        return;
    }

    timeoutCallback();
    evExit();
}


void RequestTask::timeoutCallback() {
}

