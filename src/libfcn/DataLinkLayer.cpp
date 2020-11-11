//
// Created by sdong on 2020/10/21.
//

#include "DataLinkLayer.hpp"
#include "DefaultAllocate.h"

#include "utils/ObjPool.hpp"
#include "utils/CppUtils.hpp"
#include "DataVerify.hpp"
#include "utils/Tracer.hpp"

#include <cstring>

using namespace utils;
using namespace libfcn_v2;


//ObjPool<DataLinkFrame, FCN_ALLOCATE_FRAME_NUM> framObjPool;
//
//
///* 重载New 和 Delete，以无碎片的方式进行内存分配 */
//void* DataLinkFrame::operator new(size_t size) noexcept{
//    return framObjPool.allocate();
//}
//
//void DataLinkFrame::operator delete(void * p){
//    framObjPool.deallocate(p);
//}



bool FrameIODevice::pushTxQueue(FramePtr frame){
#ifdef SYSTYPE_FULL_OS
    std::lock_guard<std::mutex> updating_lk(wr_mutex);
#endif //SYSTYPE_FULL_OS

    if(tx_frame_queue.full()){
        LOGE("frame buffer is full!!");
        return false;
    }

    tx_frame_queue.push(*frame);

    LOGV("push to frame buffer, b_cnt = %d", tx_frame_queue.size());

#ifdef SYSTYPE_FULL_OS
    write_ctrl_cv.notify_all();
#endif //SYSTYPE_FULL_OS

    return true;
}

bool FrameIODevice::sendPolling() {
#ifdef SYSTYPE_FULL_OS
    std::unique_lock<std::mutex> updating_lk(wr_mutex);
    if(tx_frame_queue.empty()){
        LOGV("frame_buffer waiting to push..");

        write_ctrl_cv.wait(updating_lk);
    }

#endif //SYSTYPE_FULL_OS


    if(tx_frame_queue.empty()){
        LOGV("frame_buffer is empty!!");
        return false;
    }

    if(sending_frame == nullptr){
        sending_frame = &tx_frame_queue.front();
    }

    if(popTxQueue(sending_frame)){
        tx_frame_queue.pop();
        sending_frame = nullptr;
        return true;
    }

    return false;
}

ByteStreamParser::ByteStreamParser(int max_buf)

    :header(ByteFrameIODevice::MAX_HEADER_LEN),
     max_buf(max_buf){

}

ByteStreamParser::ByteStreamParser(
        utils::vector_s<uint8_t>& header_,
        int max_buf):
        ByteStreamParser(max_buf){
    setHeader(header_);
}

void ByteStreamParser::setHeader(utils::vector_s<uint8_t>& header_){
    for(auto& h : header_){
        header.push_back(h);
    }
}

/*
 * 字节流解析状态机
 * */
int8_t ByteStreamParser::rxParseUpdate(uint8_t recv_byte, FramePtr recv_frame)  {
    int res = 0;

    switch (recv_state) {
        /* 接收包头0 */
        case State::HEADER0: {
            if (recv_byte == header[0]) { recv_state = State::HEADER1; }
        } break;

        /* 接收包头1 */
        case State::HEADER1: {
            /* 包头匹配，继续接收长度 */
            if (recv_byte == header[1]) {
            	recv_state = State::LEN;
            }
            /* 包头不匹配，返回等待下一个包头 */
            else {
            	recv_state = State::HEADER0;
            	res = -2;
            }
        } break;

        /* 接收包长度 */
        case State::LEN: {
            if (recv_byte < max_buf && recv_byte >= FRAME_NWK_INFO_LEN) {
                /* 长度小于缓冲区长度，且大于四个关键数据（srcID~msgID）总长度，
                 * 开始接收内容 */
                recv_frame->setFrameLen(recv_byte);

                frame_wr_ptr = recv_frame->getNetworkFramePtr();

                expect_len = recv_byte + FRAME_CRC_LEN;

                recv_state = State::NWK_FRM_CRC;

                LOGI("frame header matched");

//                /* 直接构造一个新的数据帧。智能指针可以自动释放之前未使用的数据帧，
//                 * 且保留正在使用的数据帧 */
//                receiving_frame = ESharedPtr<DataLinkFrame>(new DataLinkFrame());
//                receiving_frame->payload_len = expect_len - 4;
            } else {
                /* 长度过长或过短，认为不匹配，返回等待下一个包头 */
                recv_state = State::HEADER0;
                frame_wr_ptr = nullptr;
            }
        }
        break;

        /* 接收数据内容 */
        case State::NWK_FRM_CRC: {
            if(expect_len > 0){
                *frame_wr_ptr = recv_byte;
                frame_wr_ptr ++;
                expect_len --;
            }

            if(expect_len == 0){
                /* 已经收满，进行CRC TODO: order??*/
                recv_state = State::HEADER0;
                frame_wr_ptr = nullptr;

                uint16_t crc_result = Crc16(recv_frame->getNetworkFramePtr(),
                                            recv_frame->getNetworkFrameSize());
                uint8_t* crc_ptr = recv_frame->getCrcPtr();
                uint16_t crc_t   = crc_ptr[0];
                crc_t = crc_t<<8 | crc_ptr[1];

                if(crc_result == crc_t){
                    /* 接收成功 */
                    valid_frame_cnt++;
                    res = 1;
                }
                else {
                    res = -1;
                    error_frame_cnt++;
                    LOGW("frame recv error! CRC=%X(exp. %X)", crc_result, *(uint16_t*)crc_ptr);
                }
            }

        }break;
    }

    return res;
}

#ifdef SYSTYPE_FULL_OS
/* 互斥锁 */
#include <mutex>
#endif

//bool ByteFrameIODevice::write(DataLinkFrame* frame);


bool ByteFrameIODevice::popTxQueue(FramePtr send_frame) {
    USER_ASSERT(send_frame != nullptr);

    /* 物理层空闲时发送，如果忙则等待下次发送*/
    if(ll_byte_dev->isWriteBusy()){
        return false;
    }

    /* 添加包头 */
    send_frame->getHeaderPtr()[0] = header[0];
    send_frame->getHeaderPtr()[1] = header[1];

    /* 添加包尾校验 */
    uint16_t crc_result = Crc16(send_frame->getNetworkFramePtr(),
                                send_frame->getNetworkFrameSize());

    uint8_t* crc_ptr = send_frame->getCrcPtr();

    crc_ptr[0] = crc_result >> 8;
    crc_ptr[1] = crc_result;

    /* 发送整个数据帧
     * */
    ll_byte_dev->write(send_frame->getHeaderPtr(), send_frame->getFrameMemSize());
    return true;
}

/*
 * 获取1帧数据
 * 在上位机为阻塞式调用，只有收到1个完整的帧才会返回。永远返回true。
 *
 * 在下位机为非阻塞式调用，会在接收缓冲区中尝试找到一个完整的数据帧
 * 并返回true，或是已经清空缓冲区但无完整的数据帧，返回false。
 * 调用者有责任循环调用read直到返回false为止，以尽早清空接收
 * 缓冲区，避免数据丢失。
 *
 * 参数：frame 必须在read之前就分配好
 * */
bool ByteFrameIODevice::popRxQueue(FramePtr frame)
{
    uint8_t buf;
    uint8_t len;
    int8_t parse_res;

    while(true){
        len = ll_byte_dev->read(&buf, 1);


        if(len == 0){
            /* 异常情况，阻塞式read调用超时才返回0。否则只会返回1 */
            if(ll_byte_dev->is_blocking_recv){
                continue;
            } else{
                /* 缓冲区已经清零 */
                return false;
            }
        }

        parse_res = parser.rxParseUpdate(buf, frame);

        if(parse_res == 0){ continue; }
        else if(parse_res == 1) { return true; }
        else{ return false; }
    }
}

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

std::string libfcn_v2::frame2log(FcnFrame& frame){
#ifdef ENABLE_TRACE
    static const int BUFFER_RESERVE = 120;
    static const int BUFFER_SIZE = DATALINK_MTU * 4 + BUFFER_RESERVE;
    char buffer[BUFFER_SIZE];

    if(frame.getPayloadLen() > DATALINK_MTU){
        return std::string("::: DataLinkFrame  > DATALINK_MTU\n");
    }
    char * opcode_str = "unknown";
    if(frame.op_code < sizeof(opcode_str) / sizeof(opcode_str[0])) {
        opcode_str = mOpCodeStr[frame.op_code];
    }

    snprintf(buffer, BUFFER_SIZE - 2,
             "-----FRAME----\n"
                    " %s (0x%.2X) :  [0x%.2X]->[0x%.2X] \n"
                    " Message ID = 0x%.2X\n"
                    " Payload [%.2d] = ",

            opcode_str, frame.op_code  & 0xff,
            frame.src_id   & 0xff,
            frame.dest_id  & 0xff,

            frame.msg_id   & 0xff,
            frame.getPayloadLen());

    static int info_offset = 0;

    if(info_offset == 0){
        info_offset = strlen(buffer);
    }

    if(info_offset > BUFFER_RESERVE){
        return std::string("::: info_offset > BUFFER_RESERVE\n");
    }

    for(int i = 0; i < frame.getPayloadLen(); i ++){
        snprintf(&buffer[info_offset + i * 3],
                 BUFFER_SIZE - 2,
                 "%.2X ", frame.payload[i] & 0xff);
    }

    snprintf(&buffer[info_offset + frame.getPayloadLen() * 3],
            BUFFER_SIZE - 2,
            "\n\"%s\"\n", frame.payload);

    //TODO: cutoff
//    buffer[info_offset + frame.payload_len * 3 + frame.payload_len + 3] = '\0';

    return std::string(buffer);

#else
    return "";
#endif

}
