//
// Created by sdong on 2020/10/29.
//

#include "Log.hpp"

using namespace libfcn_v2;


utils::Tracer* Log::tracer = nullptr;

utils::Tracer* Log::getInstance(){
    if(tracer == nullptr){
        tracer = new utils::Tracer(true);

        tracer->setFilter(utils::Tracer::Level::INFO);
    }

    return tracer;
}

void Log::setLevel(LogLevel level){
    getInstance()->setFilter(level);
}

int Log::printf(utils::Tracer::Level level,
                 char *format, ...) {

    va_list arg_ptr;
    va_start(arg_ptr, format);
    int ret = getInstance()->vprintf(level, format, arg_ptr);
    va_end(arg_ptr);

    return ret;
}

static char* mOpCodeStr[]={
        (char*)"FORCE_STOP",
        (char*)"RTO_PUB",
        (char*)"RTO_REQUEST ",
        (char*)"RTO_EMERGENCY ",
        (char*)"SVO_SINGLE_READ_REQ",
        (char*)"SVO_SINGLE_READ_ACK",
        (char*)"SVO_SINGLE_WRITE_REQ",
        (char*)"SVO_SINGLE_WRITE_ACK",
        (char*)"SVO_MULTI_WRITE_START_REQ",
        (char*)"SVO_MULTI_WRITE_START_ACK",
        (char*)"SVO_MULTI_WRITE_TRANS_REQ",
        (char*)"SVO_MULTI_WRITE_TRANS_ACK",
        (char*)"SVO_MULTI_WRITE_VERIFY_REQ",
        (char*)"SVO_MULTI_WRITE_VERIFY_ACK",
        (char*)"",
};


std::string libfcn_v2::Frame2Log(DataLinkFrame& frame){
    static const int BUFFER_RESERVE = 150;

    char buffer[DATALINK_MTU * 4 + BUFFER_RESERVE];

    if(frame.payload_len > DATALINK_MTU){
        return std::string("::: DataLinkFrame  > DATALINK_MTU\n");
    }

    char * opcode_str = "";
    if(frame.op_code < sizeof(opcode_str) / sizeof(opcode_str[0])) {
        opcode_str = mOpCodeStr[frame.op_code];
    }

    sprintf(buffer, "::: DataLinkFrame \n"
                    "\tsrc   id = 0x%.2X\n"
                    "\tdest  id = 0x%.2X\n"
                    "\top code  = 0x%.2X (%s)\n"
                    "\tmsg   id = 0x%.2X\n"
                    "\tpayload[%.3d] = \n"
                    "\t\t(hex) ",
            frame.src_id   & 0xff,
            frame.dest_id  & 0xff,
            frame.op_code  & 0xff, opcode_str,
            frame.msg_id   & 0xff,
            frame.payload_len);

    static int info_offset = 0;

    if(info_offset == 0){
        info_offset = strlen(buffer);
    }

    if(info_offset > BUFFER_RESERVE){
        return std::string("::: info_offset > BUFFER_RESERVE\n");
    }

    for(int i = 0; i < frame.payload_len; i ++){
        sprintf(&buffer[info_offset + i * 3], "%.2X ", frame.payload[i] & 0xff);
    }

    sprintf(&buffer[info_offset + frame.payload_len * 3], "\n\t\t(str) %s\n", frame.payload);

    //TODO: cutoff
//    buffer[info_offset + frame.payload_len * 3 + frame.payload_len + 3] = '\0';
    return std::string(buffer);
}

std::string libfcn_v2::Frame2LogCompact(DataLinkFrame& frame){
    static const int BUFFER_RESERVE = 120;

    char buffer[DATALINK_MTU * 4 + BUFFER_RESERVE];

    if(frame.payload_len > DATALINK_MTU){
        return std::string("::: DataLinkFrame  > DATALINK_MTU\n");
    }
    char * opcode_str = "";
    if(frame.op_code < sizeof(opcode_str) / sizeof(opcode_str[0])) {
        opcode_str = mOpCodeStr[frame.op_code];
    }

    sprintf(buffer, "Frame [0x%.2X]--->[0x%.2X]\n"
                    "    Op[0x%.2X]:%s  |  Idx[0x%.2X]\n"
                    "  Data[%.3d] = ",

            frame.src_id   & 0xff,
            frame.dest_id  & 0xff,
            frame.op_code  & 0xff, opcode_str,
            frame.msg_id   & 0xff,
            frame.payload_len);

    static int info_offset = 0;

    if(info_offset == 0){
        info_offset = strlen(buffer);
    }

    if(info_offset > BUFFER_RESERVE){
        return std::string("::: info_offset > BUFFER_RESERVE\n");
    }

    for(int i = 0; i < frame.payload_len; i ++){
        sprintf(&buffer[info_offset + i * 3], "%.2X ", frame.payload[i] & 0xff);
    }

    sprintf(&buffer[info_offset + frame.payload_len * 3], "\n\"%s\"\n", frame
    .payload);

    //TODO: cutoff
//    buffer[info_offset + frame.payload_len * 3 + frame.payload_len + 3] = '\0';

    return std::string(buffer);
}
