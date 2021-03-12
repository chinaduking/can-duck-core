//
// Created by sdong on 2019/11/10.
//

#ifndef LIBFCN_REQUESTERTASK_HPP
#define LIBFCN_REQUESTERTASK_HPP

#include "DataLinkLayer.hpp"
#include "SerDesDict.hpp"
#include "EventLoop.hpp"
#include "ObjPool.hpp"

namespace can_duck{
    /* IDs */
    typedef uint64_t framets_t;

    static const int DATALINK_MTU = DATALINK_MAX_TRANS_UNIT;
    static const int MAX_NODE_NUM = MAX_LOCAL_NODE_NUM;
    static const int MAX_NODE_ID  = MAX_NODE_NUM - 1;

#pragma pack(4)
    struct ServiceFrame {
    public:
        /* 网络层信息自此开始
         * Network Layer Info */
        uint8_t src_id{0};   /* [DW1 3] 源节点ID   */
        uint8_t dest_id{0};   /* [DW1 2] 目标节点ID */

        uint8_t op_code{0};   /* [DW1 1] 操作码     */
        uint16_t msg_id{0};   /* [DW1 0] 消息ID     */

        uint8_t payload[DATALINK_MTU]{};
        uint8_t payload_len{0};

    public:
        inline uint16_t getPayloadLen() {
            return payload_len;
        }

        /* 获取指向网路数据包帧信息开头的指针 */
        inline uint8_t *getNetworkFramePtr() {
            return (uint8_t *) &src_id;
        }

        /* 获取指向网路数据包帧大小 */
        inline uint16_t getNetworkFrameSize() {
            /* 4个帧信息:
             *  src_id | dest_id | op_code | msg_id
             */
            return payload_len + 4;
        }

        /* 获取指向CRC起始的指针 */
        inline uint8_t *getCrcPtr() {
            return payload + payload_len;
        }

//        framets_t    ts_100us{ 0 };     /* 时间戳，精度为0.1ms。进行传输时，最大值为65535 */

        /* 快速数据帧拷贝
         * 因payload预留空间较大，直接赋值会造成较大CPU开销，因此只拷贝有效数据。*/
        ServiceFrame &operator=(const ServiceFrame &other) {
            this->src_id = other.src_id;
            this->dest_id = other.dest_id;
            this->op_code = other.op_code;
            this->msg_id = other.msg_id;

            emlib::memcpy(this->payload, (ServiceFrame *) &other.payload,
                          ((ServiceFrame &) other).payload_len);

            this->payload_len = other.payload_len;
            return *this;
        }

    };

    struct LinkedListNodeAllocator{
        static void* allocate(size_t size);
        static void deallocate(void* p);
    };

    using FcnEvLoop = emlib::EventLoop<ServiceFrame, LinkedListNodeAllocator>;

    class ParamServerClient;

    struct RequestCallback{
        typedef void (*Callback)(void* p_this,
                                 int ev_code, ParamServerClient* client);

        RequestCallback() = default;

        RequestCallback(Callback cb,
                        void* ctx_obj=nullptr):
                cb(cb), p_this(ctx_obj)
        {}

        inline void call(ParamServerClient* client, int ev_code){
            if(cb != nullptr){
                (*cb)(p_this, ev_code, client);
            }
        }

        Callback cb {nullptr};
        void* p_this {nullptr};
    };

    #define FCN_REQUEST_CALLBACK(fname) void fname(void* p_this, \
            int ev_code, ParamServerClient* client)

    /*
     * 抽象的数据请求过程。用于用户自定义请求的形式。
     **/
    class ParamServerRequestEv : public FcnEvLoop::Task{
    public:
        ParamServerRequestEv() : FcnEvLoop::Task(){}

        ParamServerRequestEv(
                ParamServerClient* context_client,
                ServiceFrame& frame,
                uint16_t ack_op_code,
                uint16_t timeout_ms, int retry_max=-1,
                RequestCallback&& callback=RequestCallback()):

                FcnEvLoop::Task(),

                ack_op_code(ack_op_code),
                timeout_ms(timeout_ms),
                retry_max(retry_max),

                context_client(context_client){

            cached_req = frame;

            this->callback = std::move(callback);
        }

        ~ParamServerRequestEv() override = default;

        ServiceFrame cached_req;

        void* operator new(size_t size) noexcept;
        void operator delete(void * p);

    protected:

        void onTimeout();

        /* 解析目标节点的应答数据 */
        void onRecv(ServiceFrame& frame);

        RequestCallback callback;

    private:
        uint16_t ack_op_code{0};

        uint16_t timeout_ms{2000};
        int retry_max{2};
        uint16_t retry_cnt{0};

        ParamServerClient* const context_client{nullptr};

        bool matchNotifyMsg(ServiceFrame& frame) override;
        void evUpdate() override;
        void evNotifyCallback(ServiceFrame& frame) override;
        void evTimeoutCallback() override;
    };

}

#endif //LIBFCN_REQUESTERTASK_HPP
