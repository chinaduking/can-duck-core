//
// Created by sdong on 2020/10/21.
//

#ifndef LIBFCN_V2_DATALINKLAYER_HPP
#define LIBFCN_V2_DATALINKLAYER_HPP

#include <cstdint>
#include <cstddef>

#include "LLComDevice.hpp"
//#include "utils/ESharedPtr.hpp"
#include "DefaultAllocate.h"
#include "utils/vector_s.hpp"

namespace libfcn_v2{
    /* IDs */
    typedef uint8_t frameid_t;
    static const uint16_t  MAX_NODE_NUM = 64;
    static const frameid_t MAX_NODE_ID = MAX_NODE_NUM - 1;

    /* Frame */
    typedef uint32_t framesize_t;
    typedef uint64_t framets_t;

    /* 注意：为了能快速计算CRC，选择数组长度时请保证Frame的内存对齐。*/
    static const framesize_t DATALINK_MTU = 64;

    struct DataLinkFrame{
        DataLinkFrame():
                payload_len(0),
                src_id(0), dest_id(0),
                op_code(0), msg_id(0),
                ts_100us(0)
        {}

        ~DataLinkFrame() = default;

        /* 注意，已经测试以下成员地址连续，为了能快速计算CRC，请
         * 保证内存对齐。*/

        framesize_t payload_len;       /*数据内容长度*/
        frameid_t src_id;     /* 源节点ID   */
        frameid_t dest_id;    /* 目标节点ID */
        uint8_t   op_code;    /* 操作码     */
        uint8_t   msg_id;     /* 消息ID     */
        uint8_t   payload[DATALINK_MTU]{}; /*数据内容。*/

        framets_t ts_100us;   /* 时间戳，精度为0.1ms。
                               * 进行传输时，最大值为65535
                               * （6.5535s）*/

        /* 重载New 和 Delete，以无碎片的方式进行内存分配 */
        void * operator new(size_t size) noexcept;
        void operator delete(void * p) noexcept;

        //        static std::atomic<int> cnt;
    };

}

namespace libfcn_v2{

    void buffer2Frame(DataLinkFrame* frame, uint8_t *buf, uint16_t len);
    uint16_t frame2Buffer(DataLinkFrame* frame, uint8_t *buf);


    /* 链路层设备抽象接口（一个硬件网络端口）
     *
     *
     * 链路层负责以数据帧（DataLinkFrame）为单位进行无状态的单总线网络数据传输；
     * 可保对数据包进行校验、加密、解密。在物理层可靠时，可保证数据完整可靠传输。
     * 在物理层不可靠时，不通过重传等保证可靠性。
     *
     * 不包含长数据包的拆包组包；
     * 不负责转发路由；
     * */
    class FrameIODevice{
    public:
        FrameIODevice() = default;
        virtual ~FrameIODevice() = default;

        /* 读取方法。
         * 从物理层设备接收缓冲区中直接构造一个新数据帧。缓冲区剩余部分留待下次处理。调用者有
         * 义务循环调用read，以便尽快清空硬件接收缓冲区。
         * 该方法有两种工作模式：
         *
         * 1. 非阻塞模式：
         *  从头开始遍历设备接收缓冲区，并进行数据校验。
         *  在接收到一个可通过校验的完整数据帧时，立即返回true，此时形参frame所对应的
         *  DataLinkFrame实例中即为接收到的数据帧，且该帧的数区块从设备接收缓冲区中弹出；
         *  当设备接收缓冲区已清零，但未有完整数据帧，则立即返回false。此时校验器可能保留目前
         *  解析状态，待下次调用且接收缓冲区中有新数据时继续解析。
         *
         * 2. 阻塞模式
         *  从头开始遍历设备接收缓冲区，并进行数据校验。
         *  在接收到一个可通过校验的完整数据帧时，立即返回true，此时形参frame所对应的
         *  DataLinkFrame实例中即为接收到的数据帧，且该帧数据从设备接收缓冲区中弹出；
         *  当设备接收缓冲区已清零，但未有完整数据帧，则不返回，直到接收到完整数据帧。
         *
         *
         * 注意：
         * 出于效率考虑，read方法可能并不包含新分配的DataLinkFrame实例，而是直接使用形参
         * 传入的frame。为了保证数据帧的完整性，调用者应注意以下两点：
         * 1. 调用前将frame指针指向一个提前分配好的DataLinkFrame实例；
         * 2. 在非阻塞模式下返回false时，形参frame指针不要更改。
         * */
        virtual bool read(DataLinkFrame* frame) = 0;


        /* 写入方法。
         * 将形参传入的frame转换为物理层设备需要的格式并填入设备发送缓冲区
         *
         * 该方法有两种工作模式：
         *
         * 1. 非阻塞模式
         * 当发送缓冲区满时立即返回false，否则填充缓冲区后立即返回true。物理层设备有义务
         * 保证数据帧在有限时间内成功发送。
         *
         * 对于发送缓冲区较小的物理层（如CAN，或无法使用DMA的UART/485）有必要增加软件发送队列；
         * 对于缓冲区较大的物理层（如有DMA的UART），可以不增加软件发送队列，
         * 直接把数据帧序列化到缓冲区中。
         *
         * 2. 阻塞模式
         * 阻塞等待发送完成，并返回true。
         */
        virtual bool write(DataLinkFrame* frame) = 0;

        /* 标志是否是阻塞式LL读取。 */
        bool is_blocking_recv;

        /* 最大传输长度 */
        uint32_t max_payload_size;
    };

    /*
     * 字节数据包解析状态机
     *
     * 在物理层为字节模式发送（如UART/485）的通信中，一个数据帧格式如下
     * 0x55, 0xAA, LEN, SRC_ID, DEST_ID, OP_CODE, MSG_ID, PAYLOAD, CRC_H, CRC_L
     * */
    class ByteStreamParser {
    public:
        explicit ByteStreamParser(int max_buf = DATALINK_MTU + 20);
        explicit ByteStreamParser(utils::vector_s<uint8_t>& header_,
                                  int max_buf = DATALINK_MTU + 20);
        ~ByteStreamParser() = default;

        void setHeader(utils::vector_s<uint8_t>& header_);
        int8_t parseOneByte(uint8_t new_byte, DataLinkFrame* out_frame_buf);

    private:
        enum class State {
            HEADER0 = 0,
            HEADER1,
            LEN,
            SRC_ID,
            DEST_ID,
            OP_CODE,
            MSG_ID,
            PAYLOAD,
            CRC0,
            CRC1
        };
        State recv_state { State::HEADER0 };

        utils::vector_s<uint8_t> header;

        /*  high 8b -> crc_buf[0]
         *  low  8b -> crc_buf[1] */
        uint8_t  crc_buf[2] { 0, 0 };
        uint32_t expect_len { 0 };

        int max_buf { 0 };
        int payload_recv_cnt { 0 };

        int valid_frame_cnt  { 0 };
        int error_frame_cnt  { 0 };

//        utils::ESharedPtr<DataLinkFrame> receiving_frame;

        static bool crc(DataLinkFrame *buf, uint16_t len, uint8_t *crc_out);
    };

    class ByteFrameIODevice : public FrameIODevice{
    public:
        static const int MAX_HEADER_LEN = 4;

        explicit ByteFrameIODevice(LLByteDevice* ll_byte_dev):
                ll_byte_dev(ll_byte_dev),
                header(MAX_HEADER_LEN){
            header.push_back(0x55);
            header.push_back(0xAA);
            parser.setHeader(header);
        }

        ByteFrameIODevice(LLByteDevice* ll_byte_dev,
                          utils::vector_s<uint8_t>& header):
                ByteFrameIODevice(ll_byte_dev){
            parser.setHeader(header);
        }

        ~ByteFrameIODevice() override = default;

        /* 功能定义见FrameIODevice::read */
        bool read(DataLinkFrame* frame) override;

        /* 功能定义见FrameIODevice::write */
        bool write(DataLinkFrame* frame) override;

    private:
        LLByteDevice* const ll_byte_dev;
        utils::vector_s<uint8_t> header;
        ByteStreamParser parser;
    };

#if 0
    class CanFrameIODevice : public FrameIODevice{
    public:
        CanFrameIODevice(LLCanBus* ll_can): ll_can(ll_can){
        }

        ~CanFrameIODevice() override = default;

        bool read(DataLinkFrame* frame) override;
        bool write(DataLinkFrame* frame) override;

    private:
        LLCanBus* const ll_can;

    };
#endif

}

#endif //LIBFCN_V2_DATALINKLAYER_HPP
