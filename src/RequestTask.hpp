//
// Created by sdong on 2019/11/10.
//

#ifndef LIBFCN_REQUESTERTASK_HPP
#define LIBFCN_REQUESTERTASK_HPP

#include "DataLinkLayer.hpp"
#include "utils/EventLoop.hpp"
#include "utils/ObjPool.hpp"

namespace libfcn_v2{

    struct LinkedListNodeAllocator{
        static void* allocate(size_t size);
        static void deallocate(void* p);
    };

    //TODO: LinkedList Allocator (A general allocator of Node<uniqur_ptr<ANY>>)!
    using FcnEvLoop = utils::EventLoop<DataLinkFrame, LinkedListNodeAllocator>;

    class SvoClient;
    /*
     * 抽象的数据请求过程。用于用户自定义请求的形式。
     * 由于Ninebot协议没有传输层独立的重传机制，而是将可靠传输的义务放在
     * 了协议层，因此传输层的意义是对协议层的重传机制进行抽象。
     * */
    class RequestTask : public FcnEvLoop::Task{
    public:
        RequestTask() : FcnEvLoop::Task(){}

        RequestTask(
                SvoClient* context_client,
                DataLinkFrame& frame,
                uint16_t ack_op_code,
                uint16_t timeout_ms, int retry_max=-1):

                FcnEvLoop::Task(),

                ack_op_code(ack_op_code),
                timeout_ms(timeout_ms),
                retry_max(retry_max),

                context_client(context_client){
            cached_request_frame = frame;
        }

        ~RequestTask() override = default;

        DataLinkFrame cached_request_frame;

        void* operator new(size_t size) noexcept;
        void operator delete(void * p);

    protected:

        void onTimeout();

        /* 解析目标节点的应答数据 */
        void onRecv(DataLinkFrame& frame);


        //TODO: callback!!!
        void (*callback)(void* context_obj, void* data, int ev_code) {nullptr};
        void* context_obj{nullptr};
    private:
        uint16_t ack_op_code{0};

        uint16_t timeout_ms{2000};
        int retry_max{2};
        uint16_t retry_cnt{0};

        SvoClient* const context_client{nullptr};

        bool matchNotifyMsg(DataLinkFrame& frame) override;
        void evUpdate() override;
        void evNotifyCallback(DataLinkFrame& frame) override;
        void evTimeoutCallback() override;
    };

}

#endif //LIBFCN_REQUESTERTASK_HPP
