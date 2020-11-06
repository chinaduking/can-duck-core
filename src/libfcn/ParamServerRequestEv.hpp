//
// Created by sdong on 2019/11/10.
//

#ifndef LIBFCN_REQUESTERTASK_HPP
#define LIBFCN_REQUESTERTASK_HPP

#include "DataLinkLayer.hpp"
#include "SerDesDict.hpp"
#include "EventLoop.hpp"
#include "ObjPool.hpp"

namespace libfcn_v2{

    struct LinkedListNodeAllocator{
        static void* allocate(size_t size);
        static void deallocate(void* p);
    };

    using FcnEvLoop = utils::EventLoop<DataLinkFrame, LinkedListNodeAllocator>;

    class ParamServerClient;

    struct RequestCallback{
        typedef void (*Callback)(void* ctx_obj,
                                 int ev_code, DataLinkFrame* frame);

        RequestCallback() = default;

        RequestCallback(Callback cb, void* ctx_obj=nullptr):
                cb(cb), ctx_obj(ctx_obj)
        {}

        void call(int ev_code, DataLinkFrame* frame);

        Callback cb {nullptr};
        void* ctx_obj {nullptr};
    };

    /*
     * 抽象的数据请求过程。用于用户自定义请求的形式。
     **/
    class ParamServerRequestEv : public FcnEvLoop::Task{
    public:
        ParamServerRequestEv() : FcnEvLoop::Task(){}

        ParamServerRequestEv(
                ParamServerClient* context_client,
                DataLinkFrame& frame,
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

        DataLinkFrame cached_req;

        void* operator new(size_t size) noexcept;
        void operator delete(void * p);

    protected:

        void onTimeout();

        /* 解析目标节点的应答数据 */
        void onRecv(DataLinkFrame& frame);

        RequestCallback callback;

    private:
        uint16_t ack_op_code{0};

        uint16_t timeout_ms{2000};
        int retry_max{2};
        uint16_t retry_cnt{0};

        ParamServerClient* const context_client{nullptr};

        bool matchNotifyMsg(DataLinkFrame& frame) override;
        void evUpdate() override;
        void evNotifyCallback(DataLinkFrame& frame) override;
        void evTimeoutCallback() override;
    };

}

#endif //LIBFCN_REQUESTERTASK_HPP
