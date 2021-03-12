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
    struct HeaderFastMsgExt {
        uint32_t reserve : 3;   /* =0 */
        uint32_t is_seg  : 1;   /* =0 */
        uint32_t is_msg  : 1;   /* =1 */
        uint32_t node_id : 6;
        uint32_t is_tx   : 1;
        uint32_t msg_id  : 3;
        uint32_t is_d1_empty : 1;
        uint32_t data_0  : 8;
        uint32_t data_1  : 8;
    };

    struct HeaderFastMsgStd {
        uint32_t reserve : 21;  /* =0 */
        uint32_t is_seg  : 1;   /* =0 */
        uint32_t is_msg  : 1;   /* =1 */
        uint32_t node_id : 6;
        uint32_t is_tx   : 1;
        uint32_t msg_id  : 3;
    };

    struct HeaderService{
        uint32_t reserve : 3;   /* =0 */
        uint32_t is_seg  : 1;   /* =0 */
        uint32_t is_msg  : 1;   /* =0 */
        uint32_t src_id  : 6;
        uint32_t dest_id : 6;
        uint32_t op_code : 5;
        uint32_t service_id : 10;
    };

    struct HeaderSegment{
        uint32_t reserve : 3;   /* =0 */
        uint32_t is_seg  : 1;   /* =1 */
        uint32_t src_id  : 6;
        uint32_t dest_id : 6;
        uint32_t trans_id: 6;
        uint32_t status  : 2;
        uint32_t data_0  : 8;
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
}

#endif //CAN_DUCK_COMMON_HPP
