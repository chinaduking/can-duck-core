#include "DuckDebug.hpp"
#include "Tracer.hpp"
using namespace can_duck;

#ifdef ENABLE_TRACE
static char* mOpCodeStr[]={
        (char*)"ForceStop",
        (char*)"Publish",
        (char*)"PublishReq ",
        (char*)"Emergency ",
        (char*)"ParamServer_ReadReq",
        (char*)"ParamServer_ReadAck",
        (char*)"ParamServer_WriteReq",
        (char*)"ParamServer_WriteAck",
        (char*)"SVO_MULTI_WRITE_START_REQ",
        (char*)"SVO_MULTI_WRITE_START_ACK",
        (char*)"SVO_MULTI_WRITE_TRANS_REQ",
        (char*)"SVO_MULTI_WRITE_TRANS_ACK",
        (char*)"SVO_MULTI_WRITE_VERIFY_REQ",
        (char*)"SVO_MULTI_WRITE_VERIFY_ACK",
        (char*)"",
};
#endif

uint32_t can_duck::frame2strbuf(ServiceFrame& frame, char* buffer, uint32_t buffer_size){

    if(frame.payload_len > DATALINK_MTU){
        LOGW("frame2strbuf: too long frame!");
//        return 0;
    }

    char * opcode_str = "unknown";
    if(frame.op_code < sizeof(opcode_str) / sizeof(opcode_str[0])) {
        opcode_str = mOpCodeStr[frame.op_code];
    }

    char* p_buffer = buffer;
    uint32_t buffer_remain = buffer_size;

    int info_len = snprintf(p_buffer, buffer_size - 2,
                            "-----FRAME----\n"
                            " %s (0x%.2X) :  [0x%.2X]->[0x%.2X] \n"
                            " Message ID = 0x%.2X\n"
                            " Payload [%.2d] = ",

                            opcode_str, frame.op_code  & 0xff,
                            frame.src_id   & 0xff,
                            frame.dest_id  & 0xff,

                            frame.srv_id & 0xff,
                            frame.payload_len);

    buffer_remain -= info_len;
    p_buffer += info_len;

    for(int i = 0; i < frame.payload_len; i ++){
        sprintf(p_buffer, "%.2X ", frame.payload[i]);
        p_buffer += 3;
        buffer_remain -= 3;

        if(buffer_remain < 3){
            LOGW("frame2strbuf: buffer is not enough. stop.");
            return p_buffer - buffer;
        }
    }

    info_len = snprintf(p_buffer, buffer_remain-1, "\n\"%s\"\n", frame.payload);
    buffer_remain -= info_len;

    p_buffer += info_len;

    return p_buffer - buffer;
}


std::string can_duck::frame2stdstr(ServiceFrame& frame){
#ifdef ENABLE_TRACE

    static const int BUFFER_RESERVE = 150;
    static const int BUFFER_SIZE = DATALINK_MTU * 3 + BUFFER_RESERVE;
    char buffer[BUFFER_SIZE];

    int used_size = frame2strbuf(frame, buffer, BUFFER_SIZE);
    //TODO: cutoff
//    buffer[info_offset + frame.payload_len * 3 + frame.payload_len + 3] = '\0';

    return std::string(buffer);

#else
    return "";
#endif

}//
// Created by 董世谦 on 2021/3/12.
//

