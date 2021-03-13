//
// Created by sdong on 2019/11/10.
//

#ifndef CANDUCK_CLIENTREQUESTEV_HPP
#define CANDUCK_CLIENTREQUESTEV_HPP

#include "SerDesDict.hpp"
#include "EventLoop.hpp"
#include "ObjPool.hpp"
#include "Common.hpp"


namespace can_duck{

    struct UPtrListAllocator{
        static void* allocate(size_t size);
        static void deallocate(void* p);
    };

    using FcnEvLoop = emlib::EventLoop<ServiceFrame, UPtrListAllocator>;

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

    #define DUCK_REQUEST_CALLBACK(fname) void fname(void* p_this, \
            int ev_code, ParamServerClient* client)

    /*
     * 抽象的数据请求过程。用于用户自定义请求的形式。
     **/
    class ClientRequestEv : public FcnEvLoop::Task{
    public:
        ClientRequestEv() : FcnEvLoop::Task(){}

        ClientRequestEv(
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

        ~ClientRequestEv() override = default;

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

#endif //CANDUCK_CLIENTREQUESTEV_HPP
