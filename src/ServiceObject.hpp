//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_SERVICEOBJECT_HPP
#define LIBFCN_V2_SERVICEOBJECT_HPP

#include "RealtimeObject.hpp"

namespace libfcn_v2 {

    #define SVO_RW_PREV_MASK (0x01 << 0)
    #define SVO_RD_STAT_MASK (0x06 << 1)
    #define SVO_WR_STAT_MASK (0x06 << 4)

    #define USE_EVLOOP

//    template<class T>
//    using SvoDictItem = ObjDictItemCb<T>;

    template<class T>
    struct SvoDictItem : public ObjDictItemCb<T>{
        /* 标志是服务器端还是客户端。
         * 如果是服务器端，只响应请求帧；如果是客户端，只响应应答帧。
         * 由Client/Server的构造函数进行置位。*/
        bool is_server {false};
    };

    class SvoDict : public ObjectDict {
    public:
        SvoDict(uint16_t dict_size) : ObjectDict(dict_size) {}
        ~SvoDict() override = default;

    protected:

    };

    class SvoClient{
    public:
        SvoClient();
        ~SvoClient();

        void  readUnblocking(ObjDictItemBase& item, FcnCallbackInterface callback);
        void writeUnblocking(ObjDictItemBase& item, FcnCallbackInterface callback);

#ifdef SYSTYPE_FULL_OS
        void  readBlocking(ObjDictItemBase& item);
        void writeBlocking(ObjDictItemBase& item);
#endif
        uint16_t server_addr { 0 };

    private:
        SvoDict* svo_dict{nullptr};
    };

    class SvoServer{

    };

}

#endif //LIBFCN_V2_SERVICEOBJECT_HPP
