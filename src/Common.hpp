//
// Created by 董世谦 on 2021/3/13.
//

#ifndef CAN_DUCK_COMMON_HPP
#define CAN_DUCK_COMMON_HPP

#include "CppUtils.hpp"

namespace can_duck{
    /* IDs */
    typedef uint64_t framets_t;

    static const int DATALINK_MTU = 64;
    static const int MAX_NODE_NUM = 64;
    static const int MAX_NODE_ID  = MAX_NODE_NUM - 1;


    /* --------------------------------------------------------- */
    /* 注意：内存中为LSB-First，ID中为MSB-First。因此包头位段的定义顺序
     * 必须为倒序！！*/

    /*
     * 基于29位扩展ID的快实时消息包头
     * */
    struct HeaderFastMsgExt {
        uint32_t data_1  : 8;   /* 数据1 */
        uint32_t data_0  : 8;   /* 数据0 */
        uint32_t is_d1_empty : 1;   /* data_1是否是空的 */
        uint32_t msg_id  : 3;   /* 消息ID */
        uint32_t is_tx   : 1;   /* 是否是所有者发送的数据 */
        uint32_t node_id : 6;   /* 所有者节点ID */
        uint32_t is_msg  : 1;   /* 是否是实时消息 =1 */
        uint32_t is_seg  : 1;   /* 是否是分段数据包 =0 */
        uint32_t reserve : 3;   /* 保留位 =0 */
    };


    /*
     * 基于11位扩展ID的快实时消息包头
     **/
    struct HeaderFastMsgStd {
        uint32_t msg_id  : 4;   /* 消息ID */
        uint32_t is_tx   : 1;   /* 是否是所有者发送的数据 */
        uint32_t node_id : 6;   /* 所有者节点ID */

        uint32_t reserve : 21;  /* 保留位 =0 */
    };

    /*
     * 服务数据包
     **/
    struct HeaderService{
        uint32_t service_id : 10;   /* 服务ID */
        uint32_t op_code : 5;   /* 操作码 */
        uint32_t dest_id : 6;   /* 目标节点ID */
        uint32_t src_id  : 6;   /* 源节点ID */
        uint32_t is_msg  : 1;   /* 是否是实时消息 =0 */
        uint32_t is_seg  : 1;   /* 是否是分段数据包 =0 */
        uint32_t reserve : 3;   /* 保留位 =0 */
    };



    /*分段数据包*/
    struct HeaderSegment{
        uint32_t data_0  : 8;   /* 数据0 */
        uint32_t status  : 2;   /* 0: 启动分段传输, 1: 正在传输, 2: 传输结束 */
        uint32_t trans_id: 6;   /* 传输计数，倒数 */
        uint32_t dest_id : 6;   /* 目标节点ID */
        uint32_t src_id  : 6;   /* 源节点ID */
        uint32_t is_seg  : 1;   /* 是否是分段数据包 =1 */
        uint32_t reserve : 3;   /* 保留位 =0 */
    };

    /**
     * 服务数据帧
     */
    struct ServiceFrame {
        uint8_t  src_id{0};    /* 源节点ID   */
        uint8_t  dest_id{0};   /* 目标节点ID */

        uint8_t  op_code{0};   /* 操作码     */
        uint16_t srv_id{0};    /* 服务ID     */

        uint8_t payload[DATALINK_MTU]{};
        uint8_t payload_len{0};
//      framets_t    ts_100us{ 0 };     /* 时间戳，精度为0.1ms。进行传输时，最大值为65535 */

        /* 快速数据帧拷贝
         * 因payload预留空间较大，直接赋值会造成较大CPU开销，因此只拷贝有效数据。*/
        ServiceFrame &operator=(const ServiceFrame &other) {
            this->src_id = other.src_id;
            this->dest_id = other.dest_id;
            this->op_code = other.op_code;
            this->srv_id = other.srv_id;

            emlib::memcpy(this->payload, (ServiceFrame *) &other.payload,
                          ((ServiceFrame &) other).payload_len);

            this->payload_len = other.payload_len;
            return *this;
        }

    };


    /*
     * 最多支持的本地节点数目
     **/
    #define MAX_LOCAL_NODE_NUM 6

    /*
     * 是否使用事件循环管理请求
     **/
    #define USE_REQUEST_EVLOOP


    /*
     * 一个客户端可以同时发起的请求
     **/
    #define CLIENT_MAX_REQ_NUM 5


    /*
    * 最多可在堆上创建的数据帧
    **/
    #define FCN_ALLOCATE_FRAME_NUM 4

    /*
    *
    **/
    #define DATALINK_MAX_TRANS_UNIT  64

    /*
    * 最多支持的通信端口数目
    * */
    #define MAX_COM_PORT_NUM 4




    /*
    *
    **/
    #define ENABLE_LOG
}

#endif //CAN_DUCK_COMMON_HPP
