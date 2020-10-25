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
    /*参数表任务状态：包括读、写*/
    enum ParamOperateStatus : uint8_t {
        PARAM_STATUS_IDLE = 0, /*初始状态/无任务状态*/
        PARAM_STATUS_PENDDING, /*正在进行读写*/
        PARAM_STATUS_SUCCESS,  /*读写成功*/
        PARAM_STATUS_REJECTED, /*访问被拒绝（可能原因：服务器上的元信息和数据包不匹配、没有权限）*/
        PARAM_STATUS_TIMEOUT,  /*访问超时（可能原因：服务器上的元信息和数据包不匹配、网络层通信失败）*/
        PARAM_STATUS_UNKNOWN   /*未知错误*/
    };

    template<class T>

    struct SvoDictItem : public ObjDictItemBase{
        SvoDictItem(index_t index,
                    data_size_t data_size)
                :
                ObjDictItemBase(index, data_size)
        {
            is_server = 0;
            wr_access = 0;
            read_status = PARAM_STATUS_IDLE;
            write_status = PARAM_STATUS_IDLE;
        }

        /*
         * 取得子类数据对象。无回调，则子类必须将数据放在第一个成员；有回调，则放在回调对象之后
         */
        inline uint8_t* getDataPtr(){
            return ((uint8_t*)this) + sizeof(SvoDictItem);
        }

        /*
         * 回调对象（如果不支持回调则返回空指针）
         */
        FcnCallbackInterface* callback {nullptr};


        uint8_t is_server    : 1;
        uint8_t wr_access    : 1;

        /* 客户端 读取任务状态（用户只能通过fetchStatus方法进行只读访问）*/
        uint8_t read_status  : 3;

        /* 客户端 写入任务状态（用户只能通过fetchStatus方法进行只读访问）**/
        uint8_t write_status : 3;
    };



    class SvoClient{
    public:
        SvoClient();
        ~SvoClient();

        void  readUnblocking(RtoDictItemBase& item, FcnCallbackInterface callback);
        void writeUnblocking(RtoDictItemBase& item, FcnCallbackInterface callback);

#ifdef SYSTYPE_FULL_OS
        void  readBlocking(RtoDictItemBase& item);
        void writeBlocking(RtoDictItemBase& item);
#endif
        uint16_t server_addr { 0 };

    private:
        ObjectDict* svo_dict{nullptr};
    };

    class SvoServer{

    };

}

#endif //LIBFCN_V2_SERVICEOBJECT_HPP
