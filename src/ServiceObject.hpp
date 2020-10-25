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



#pragma pack(2)
    /*
     * 对象字典（Object Dictionary）成员.
     * 内存按2Byte对齐，更改时要注意, sizeof(ObjDictItemBase) = 2
     * */
    class ServiceObjectBase{
    public:
        ServiceObjectBase(obj_idx_t index,
                           obj_size_t data_size,
                           bool wr_access=false)
                :
                index(index),
                data_size(data_size),

                is_server(0),
                wr_access(wr_access),
                read_status(PARAM_STATUS_IDLE),
                write_status(PARAM_STATUS_IDLE){
            USER_ASSERT(data_size <= MAX_OBJ_SZIE);
        }

        virtual ~ServiceObjectBase() = default;


        /* 消息索引 */
        const obj_idx_t index{0};

        /* 消息数据大小，最长128字节。不支持变长 */
        const obj_size_t data_size{0};

        uint8_t is_server    : 1;
        uint8_t wr_access    : 1;

        /* 客户端 读取任务状态（用户只能通过fetchStatus方法进行只读访问）*/
        uint8_t read_status  : 3;

        /* 客户端 写入任务状态（用户只能通过fetchStatus方法进行只读访问）**/
        uint8_t write_status : 3;

        FcnCallbackInterface* callback{nullptr};

        static const int MAX_OBJ_SZIE = 0xFF;

        /*
         * 取得子类数据对象。无回调，则子类必须将数据放在第一个成员；有回调，则放在回调对象之后
         */
        inline uint8_t* getDataPtr(){
            return ((uint8_t*)this) + sizeof(ServiceObjectBase);
        }
    };

    /*
     * 实时数据对象（Real-Time Object）字典成员
     * 实现了类型安全的数据存储。
     * */

    /*
     * 不支持回调的字典项目
     * */
    template <typename T>
    struct ServiceObject : public ServiceObjectBase{
        explicit ServiceObject(obj_idx_t index, bool wr_access):
                ServiceObjectBase(index, sizeof(T), wr_access){}

        void operator<<(T input) { data = input; }
        void operator>>(T &input) { input = data; }

        T data;
    };

#pragma pack(0)



    class RealtimeObjectDict{

    public:
        explicit RealtimeObjectDict(obj_idx_t dict_size) : obj_dict(dict_size){
            obj_dict.resize(dict_size);
        }

        virtual ~RealtimeObjectDict()  = default;


        /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
        obj_size_t singleWrite(obj_idx_t index, uint8_t *data, obj_size_t len);


        /*默认字段
         * TODO: 版本校验？
         * 1. 可手动校验：使用快照指令
         * 2. 在网络配置阶段，使用专用协议读取*/
//        RtoDictItemNoCb<uint32_t> version;


        /* 自定义写入一个项目后的动作（回调/置标志位等） */
        void writePostAction(obj_idx_t& index){};


        utils::vector_s<RealtimeObjectBase*> obj_dict;
    };


    class SvoClient{
    public:
        SvoClient();
        ~SvoClient();

        void  readUnblocking(RealtimeObjectBase& item, FcnCallbackInterface callback);
        void writeUnblocking(RealtimeObjectBase& item, FcnCallbackInterface callback);

#ifdef SYSTYPE_FULL_OS
        void  readBlocking(RealtimeObjectBase& item);
        void writeBlocking(RealtimeObjectBase& item);
#endif
        uint16_t server_addr { 0 };

    private:
        RealtimeObjectDict* svo_dict{nullptr};
    };

    class SvoServer{

    };

}

#endif //LIBFCN_V2_SERVICEOBJECT_HPP
