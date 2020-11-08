//
// Created by sdong on 2020/10/21.
//

#include "DataLinkLayer.hpp"
#include "DefaultAllocate.h"

#include "utils/ObjPool.hpp"
#include "utils/CppUtils.hpp"
#include "DataVerify.hpp"
#include "Tracer.hpp"

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
    //TODO: con_var to trigger send!!

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

bool ByteStreamParser::crc(FramePtr buf, uint16_t len, uint8_t *crc_out) {
    /* 因Frame成员在内存里地址连续，故可以这样操作。
     * 注意不要改变DataLinkFrame的内存布局
     * 跳过第一个长度信息不计算。*/
    uint16_t crc_result = Crc16((uint8_t*)&(buf->src_id), len);
    uint16_t crc_t = crc_out[0];
    crc_t = crc_t<<8 | crc_out[1];

    if(crc_result == crc_t)
        return true;
    else
        return false;
}


/*
 * 字节流解析状态机
 * */
int8_t ByteStreamParser::parseOneByte(uint8_t new_byte, FramePtr out_frame_buf)  {
    int res = 0;

    switch (recv_state) {
        /* 接收包头0 */
        case State::HEADER0: {
            if (new_byte == header[0]) { recv_state = State::HEADER1; }
        } break;

        /* 接收包头1 */
        case State::HEADER1: {
            /* 包头匹配，继续接收长度 */
            if (new_byte == header[1]) { recv_state = State::LEN; }
            /* 包头不匹配，返回等待下一个包头 */
            else { recv_state = State::HEADER0; }
        } break;

        /* 接收包长度 */
        case State::LEN: {
            if (new_byte < max_buf && new_byte >= 4) {
                /* 长度小于缓冲区长度，且大于四个关键数据（srcID~msgID）总长度，
                 * 开始接收内容 */
                recv_state = State::SRC_ID;
                expect_len = new_byte;

//                /* 直接构造一个新的数据帧。智能指针可以自动释放之前未使用的数据帧，
//                 * 且保留正在使用的数据帧 */
//                receiving_frame = ESharedPtr<DataLinkFrame>(new DataLinkFrame());
//                receiving_frame->payload_len = expect_len - 4;

                out_frame_buf->payload_len = expect_len - 4;

            } else {
                /* 长度过长或过短，认为不匹配，返回等待下一个包头 */
                recv_state = State::HEADER0;
            }
        }
        break;

        case State::SRC_ID:{ /* 接收源地址 */
            out_frame_buf->src_id = new_byte; recv_state = State::DEST_ID;
        }break;

        case State::DEST_ID:{ /* 接收目标地址 */
            out_frame_buf->dest_id = new_byte; recv_state = State::OP_CODE;
        }break;


        case State::OP_CODE:{  /* 接收操作码 */
            out_frame_buf->op_code = new_byte; recv_state = State::MSG_ID;
        }break;

        case State::MSG_ID:{ /* 接收消息ID */
            out_frame_buf->msg_id = new_byte;  recv_state = State::PAYLOAD;
            payload_recv_cnt = 0;
        }break;

        case State::PAYLOAD: {/* 接收数据内容 */
            out_frame_buf->payload[payload_recv_cnt] = new_byte;
            payload_recv_cnt++;

            /* 已经收满，即将进行CRC */
            if (payload_recv_cnt >= out_frame_buf->payload_len) {
                recv_state = State::CRC0;
            }
        }break;

        case State::CRC0: {
            crc_buf[0] = new_byte; recv_state = State::CRC1;
        } break;

        case State::CRC1: {
            crc_buf[1] = new_byte;
            recv_state = State::HEADER0;
            if (crc(&(*out_frame_buf), expect_len, crc_buf)) {
//                /* 接收成功，推入接收队列 */
//                frame = receiving_frame;
                valid_frame_cnt++;
                res = 1;
            } else {
                res = -1;
                error_frame_cnt++;
            }

            recv_state = State::HEADER0;
        } break;
    }

    return res;
}

#ifdef SYSTYPE_FULL_OS
/* 互斥锁 */
#include <mutex>
#endif

//bool ByteFrameIODevice::write(DataLinkFrame* frame);


bool ByteFrameIODevice::popTxQueue(FramePtr frame) {
    USER_ASSERT(frame != nullptr);

    switch (send_state) {
        case SendState::Idle:
            LOGV("get a new frame!!");
            send_state = SendState::Header;

        /* 发送包头 + 长度*/
        case SendState::Header:
            /* 这里的数据长度是串口作为物理层的"模拟数据包"的长度（即包头-包尾CRC直接所有数据
             * 的字节数）。等于payload_len+src_id+dest_id+msg_id+opcode*/
            header_buf[header.size()] = frame->payload_len + 4;

            /* 这里的包头指串口作为物理层的"模拟数据包"*/
            if(!ll_byte_dev->isWriteBusy()){
                /* 物理层空闲时发送，如果忙则等待下次发送*/
                ll_byte_dev->write(header_buf, header.size()+1);
                send_state = SendState::Frame;
            }
            return false;
//            break;

        /* 发送数据帧.这里的包头指串口作为物理层的"模拟数据包" */
        case SendState::Frame:
            /* */
            if(!ll_byte_dev->isWriteBusy()){
                /* 物理层空闲时发送，如果忙则等待下次发送
                 * 因DatalinkFrame从src_id开始地址连续，因此可以作为buffer直接发送*/
                ll_byte_dev->write(&frame->src_id,
                                   frame->payload_len + 4);

                /* 因Frame成员在内存里地址连续，故可以这样操作。
                 * 注意不要改变DataLinkFrame的内存布局
                 * 跳过第一个长度信息不计算。
                 *
                 * 注意：在Frame状态里计算CRC可以只计算一次*/
                uint16_t crc_result = Crc16(&frame->src_id,
                                            frame->payload_len + 4);

                crc_buf[0] = crc_result >> 8;
                crc_buf[1] = crc_result;

                send_state = SendState::Crc;
            }
            return false;
//            break;

        /* 发送包尾。指串口作为物理层的"模拟数据包"*/
        case SendState::Crc:
            if(!ll_byte_dev->isWriteBusy()){
                /* 物理层空闲时发送，如果忙则等待下次发送*/
                ll_byte_dev->write(crc_buf,
                                   2);
                /* 发送成功后，弹出正在发送的数据帧 */

                LOGV("sent frame!!");

                send_state = SendState::Idle;
                return true;
            }
            return false;
//            break;
    }
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

        parse_res = parser.parseOneByte(buf, frame);

        switch(parse_res) {
            case 0:
                continue;
            case 1:
                return true;
            case -1:
                return false;
            default:
                break;
        }
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

std::string libfcn_v2::frame2log(DataLinkFrame& frame){
#ifdef ENABLE_TRACE
    static const int BUFFER_RESERVE = 120;

    char buffer[DATALINK_MTU * 4 + BUFFER_RESERVE];

    if(frame.payload_len > DATALINK_MTU){
        return std::string("::: DataLinkFrame  > DATALINK_MTU\n");
    }
    char * opcode_str = "unknown";
    if(frame.op_code < sizeof(opcode_str) / sizeof(opcode_str[0])) {
        opcode_str = mOpCodeStr[frame.op_code];
    }

    sprintf(buffer, "-----FRAME----\n"
                    " %s (0x%.2X) :  [0x%.2X]->[0x%.2X] \n"
                    " Message ID = 0x%.2X\n"
                    " Payload [%.2d] = ",

            opcode_str, frame.op_code  & 0xff,
            frame.src_id   & 0xff,
            frame.dest_id  & 0xff,

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

#else
    return "";
#endif

}
