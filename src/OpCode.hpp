//
// Created by sdong on 2020/10/22.
//

#ifndef LIBFCN_V2_OPCODE_HPP
#define LIBFCN_V2_OPCODE_HPP

#include <cstdint>

namespace libfcn_v2{

    enum class OpCode : uint8_t {
        /* 紧急停止 */
        FORCE_STOP           = 0x00,


        /* -------------------------------------------------
         *               实时对象
         * -------------------------------------------------
         */
        /* 实时对象发布，在未配置转发规则时，仅限本地广播 */
        RTO_PUB              = 0x01,

        /* 请求一次实时对象发布 */
        RTO_REQUEST          = 0x02,

        /* 紧急事件发布，不进行频率控制，不论转发规则，都进行全网广播 */
        RTO_EMERGENCY        = 0x03,


        /* -------------------------------------------------
         *           服务对象，单帧收发
         * -------------------------------------------------
         */
        /* 服务对象单帧读请求 */
        SVO_SINGLE_READ_REQ  = 0x04,

        /* 服务对象单帧读应答 */
        SVO_SINGLE_READ_ACK  = 0x05,

        /* 服务对象单帧写请求 */
        SVO_SINGLE_WRITE_REQ = 0x06,

        /* 服务对象单帧写应答 */
        SVO_SINGLE_WRITE_ACK = 0x07,



        /* -------------------------------------------------
         *           服务对象，多帧收发
         * -------------------------------------------------
         */
        /* 服务对象多帧写，起始请求 */
        SVO_MULTI_WRITE_START_REQ  = 0x08,

        /* 服务对象多帧写，起始应答 */
        SVO_MULTI_WRITE_START_ACK  = 0x09,

        /* 服务对象多帧写，传输请求 */
        SVO_MULTI_WRITE_TRANS_REQ  = 0x0A,

        /* 服务对象多帧写，传输应答 */
        SVO_MULTI_WRITE_TRANS_ACK  = 0x0B,

        /* 服务对象多帧写，校验请求 */
        SVO_MULTI_WRITE_VERIFY_REQ = 0x0C,

        /* 服务对象多帧写，校验应答 */
        SVO_MULTI_WRITE_VERIFY_ACK = 0x0D,



        /* -------------------------------------------------
         *           CAN总线分包发送
         * -------------------------------------------------
         */
        MULTIFRAME_PAYLOAD   = 0x0E
    };

//
//    /* 操作码定义 */
//    typedef uint8_t opcode_t;
//
//
//    static const opcode_t OPCODE_HEARTBEAT            = 0x0C;
//
//    static const opcode_t OPCODE_ERROR_REPORT         = 0x0D;
//    static const opcode_t OPCODE_ERROR_REPORT_ACK     = 0x0E;
//
//
//    /* 扩展操作码，采用消息ID作为第二个操作码 */
//    static const opcode_t OPCODE_EXTENDED_OPCODE      = 0x10;
//
//    /* 动作 */
//    static const opcode_t OPCODE_ACTION_REQUEST       = 0x11;
//    static const opcode_t OPCODE_ACTION_RESPONSE      = 0x12;
//    static const opcode_t OPCODE_ACTION_END_REQUEST   = 0x13;
//    static const opcode_t OPCODE_ACTION_END_RESPONSE  = 0x14;


}

#endif //LIBFCN_V2_OPCODE_HPP
