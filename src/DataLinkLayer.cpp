//
// Created by sdong on 2020/10/21.
//

#include "DataLinkLayer.hpp"
#include "DefaultAllocate.h"

#include "utils/ObjPool.hpp"
#include "utils/CppUtils.hpp"
#include "utils/DataVerify.hpp"

using namespace utils;
using namespace libfcn_v2;

ObjPool<DataLinkFrame, FCN_ALLOCATE_FRAME_NUM> framObjPool;


/* 重载New 和 Delete，以无碎片的方式进行内存分配 */
void* DataLinkFrame::operator new(size_t size) noexcept{
    return framObjPool.allocate();
}

void DataLinkFrame::operator delete(void * p){
    framObjPool.deallocate(p);
}

void libfcn_v2::buffer2Frame(DataLinkFrame* frame, uint8_t *buf, uint16_t len) {
    if(nullptr == buf)
        return ;

    uint16_t i = 0;
    frame->payload_len = len - 4; /* remove op_code and msg_id */
    frame->src_id      = buf[i++];
    frame->dest_id     = buf[i++];
    frame->op_code     = buf[i++];
    frame->msg_id      = buf[i++];

    utils::memcpy(frame->payload, buf+i, frame->payload_len);

    i += frame->payload_len;

    /*No timestamp */
    if(i+8 > len)
        return;
    /*TODO: timestamp */
//    for(uint8_t j=0; j<8; j++){
//        frame->timestamp <<= 8;
//        frame->timestamp |=buf[i++];
//    }
}


uint16_t libfcn_v2::frame2Buffer(DataLinkFrame* frame, uint8_t *buf) {
    if(buf == nullptr){
        return 0;
    }

    uint16_t i=0;
    buf[i++] = (uint8_t)(frame->payload_len + 4);
    buf[i++] = (uint8_t)(frame->src_id);
    buf[i++] = (uint8_t)(frame->dest_id);
    buf[i++] = (uint8_t)(frame->op_code);
    buf[i++] = (uint8_t)(frame->msg_id);

    utils::memcpy(buf+i, frame->payload, frame->payload_len);
    i += frame->payload_len;

    /*TODO: timestamp */
//    for(uint8_t j=0; j<8; j++){
//        buf[i++] = (uint8_t)(frame->timestamp>>(56-j*8));
//    }

    return i;
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

bool ByteStreamParser::crc(DataLinkFrame *buf, uint16_t len, uint8_t *crc_out) {
    /* 因Frame成员在内存里地址连续，故可以这样操作 */
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
int8_t ByteStreamParser::parseOneByte(uint8_t new_byte, DataLinkFrame* out_frame_buf)  {
    int res = 0;

    switch (recv_state) {
        /* 接收包头0 */
        case State::HEADER0: {
            if (new_byte == header[0]) {
                recv_state = State::HEADER1;
            }
        }
            break;


            /* 接收包头1 */
        case State::HEADER1: {
            /* 包头匹配，继续接收长度 */
            if (new_byte == header[1]) {
                recv_state = State::LEN;
            } else {
                /* 包头不匹配，返回等待下一个包头 */
                recv_state = State::HEADER0;
            }
        }
            break;

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
            /* 接收源地址 */
        case State::SRC_ID: {
            out_frame_buf->src_id = new_byte;
            recv_state = State::DEST_ID;
        }
            break;

            /* 接收目标地址 */
        case State::DEST_ID: {
            out_frame_buf->dest_id = new_byte;
            recv_state = State::OP_CODE;
        }
            break;

            /* 接收操作码 */
        case State::OP_CODE: {
            out_frame_buf->op_code = new_byte;
            recv_state = State::MSG_ID;
        }
            break;

            /* 接收消息ID */
        case State::MSG_ID: {
            out_frame_buf->msg_id = new_byte;
            recv_state = State::PAYLOAD;
            payload_recv_cnt = 0;
        }
            break;

            /* 接收数据内容 */
        case State::PAYLOAD: {
            out_frame_buf->payload[payload_recv_cnt] = new_byte;
            payload_recv_cnt++;

            /* 已经收满，即将进行CRC */
            if (payload_recv_cnt >= out_frame_buf->payload_len) {
                recv_state = State::CRC0;
            }
        }
            break;

        case State::CRC0: {
            crc_buf[0] = new_byte;
            recv_state = State::CRC1;
        }
            break;
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
        }
            break;
    }

    return res;
}


bool ByteFrameIODevice::write(DataLinkFrame* frame){
    uint8_t* p_buf = ll_byte_dev->write_buf;

    assert(ll_byte_dev->write_buf_size >= header.size() + DATALINK_MTU + 2);

    *p_buf = header[0];  p_buf ++;
    *p_buf = header[1];  p_buf ++;

    uint16_t len = frame2Buffer(&*frame, p_buf);
    uint16_t crc = Crc16(p_buf + 1, len - 1);

    p_buf += len;
    *p_buf = (uint8_t)(crc >> 8); p_buf ++;
    *p_buf = (uint8_t) crc;       p_buf ++;

    return ll_byte_dev->write(ll_byte_dev->write_buf, len + 4);
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
bool ByteFrameIODevice::read(DataLinkFrame* frame)
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


