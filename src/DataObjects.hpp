//
// Created by sdong on 2020/10/26.
//

#ifndef LIBFCN_V2_DATAOBJECTS_HPP
#define LIBFCN_V2_DATAOBJECTS_HPP

#include <cstdint>
#include "utils/CppUtils.hpp"
#include "utils/vector_s.hpp"


namespace libfcn_v2 {

    /* 数据长度标志位为无符号8位整形，最大255
 * */
    typedef uint8_t obj_idx_t;
    typedef uint8_t obj_size_t;
    typedef uint16_t rto_timestamp_t;

    /*非阻塞式任务的回调函数*/
    struct FcnCallbackInterface {
        virtual void callback(void *data, uint8_t ev_code) = 0;
    };
}


/* ---------------------------------------------------------
 *               Realtime Object Definition
 * ---------------------------------------------------------
 */
namespace libfcn_v2 {
#pragma pack(2)
    /*
     * 对象字典（Object Dictionary）成员.
     * 内存按2Byte对齐，更改时要注意, sizeof(ObjDictItemBase) = 2
     * */
    struct RealtimeObjectBase{
        RealtimeObjectBase(obj_idx_t index,
                           obj_size_t data_size,
                           bool derived_has_callback=false)
                :
                index(index),
                derived_has_callback(derived_has_callback),
                data_size(data_size){
            USER_ASSERT(data_size <= MAX_OBJ_SZIE);
        }


        /* 消息索引 */
        const obj_idx_t index{0};

        /* 子类字典成员是否含有回调对象（TransferCallbackPtr）的标志位
         * （为了节省函数指针的内存） */
        const obj_size_t derived_has_callback : 1;


        /* 消息数据大小，最长128字节。不支持变长 */
        const obj_size_t data_size : 7;

        static const int MAX_OBJ_SZIE = 64;

        rto_timestamp_t timestamp_01ms{0};

        /*
         * 取得子类数据对象。无回调，则子类必须将数据放在第一个成员；有回调，则放在回调对象之后
         */
        inline void* getDataPtr(){
            if(!derived_has_callback){
                return ((uint8_t*)this) + sizeof(RealtimeObjectBase);
            } else{
                return ((uint8_t*)this) + sizeof(RealtimeObjectBase) +
                                        sizeof(FcnCallbackInterface);
            }
        }

        /*
         * 取得子类回调对象（如果不支持回调则返回空指针）
         */
        inline FcnCallbackInterface* getCallbackPtr(){
            if(!derived_has_callback){
                return nullptr;
            } else{
                /* 由括号内向括号外：
                 * 1. 取得存储回调地址的指针的地址。
                 * 2. 按指针的数据类型解引用，取得指针所指的回调的地址。
                 * 3. 根据回调地址构造指向回调的指针，并返回。
                 * */
                return (FcnCallbackInterface*)(*(uint64_t*)(
                        (uint8_t*)this + sizeof(RealtimeObjectBase)));
            }
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
    struct RealtimeObjectNoCb : public RealtimeObjectBase{
        explicit RealtimeObjectNoCb(obj_idx_t index):
                RealtimeObjectBase(index, sizeof(T), false){}

        void operator<<(T input) { data = input; }
        void operator>>(T &input) { input = data; }

        T data;
    };


    /*
     * 支持回调的字典项目
     * */
    template <typename T>
    struct RealtimeObjectCb : public RealtimeObjectBase{
        explicit RealtimeObjectCb(obj_idx_t index):
                RealtimeObjectBase(index, sizeof(T), true){}

        void operator<<(T input) { data = input; }
        void operator>>(T &input) { input = data; }

        FcnCallbackInterface* callback{nullptr};
        T data;
    };

#pragma pack(0)


    struct RealtimeObjectDict{
        RealtimeObjectDict(obj_idx_t dict_size) : obj_dict(dict_size){
            obj_dict.resize(dict_size);
        }

//        virtual ~RealtimeObjectDict()  = default;

        /*默认字段
         * TODO: 版本校验？
         * 1. 可手动校验：使用快照指令
         * 2. 在网络配置阶段，使用专用协议读取*/
//        RtoDictItemNoCb<uint32_t> version;

        utils::vector_s<RealtimeObjectBase*> obj_dict;
    };

}


/* ---------------------------------------------------------
 *               Service Object Definition
 * ---------------------------------------------------------
 */
namespace libfcn_v2{

    /*参数表任务状态：包括读、写*/
    enum class SvoClientStat : uint8_t {
        Idle = 0,   /*初始状态/无任务状态*/
        Pendding,   /*正在进行读写*/
        Ok,         /*读写成功*/
        Rejected,   /*访问被拒绝（可能原因：服务器上的元信息和数据包不匹配、没有权限）*/
        Timeout,    /*访问超时（可能原因：网络层通信失败）*/
        Unknown     /*未知错误*/
    };


#pragma pack(2)
    /*
     * 对象字典（Object Dictionary）成员.
     * 内存按2Byte对齐，更改时要注意, sizeof(ServiceObjectBase) = 4
     * */
    struct ServiceObjectBase{
        ServiceObjectBase(obj_idx_t index,
                          obj_size_t data_size,
                          bool wr_access=false)
                :
                index(index),
                data_size(data_size),

                is_server(0),
                wr_access(wr_access),
                read_status(static_cast<uint8_t>
                            (SvoClientStat::Idle)),
                write_status(static_cast<uint8_t>
                             (SvoClientStat::Idle)){
            USER_ASSERT(data_size <= MAX_OBJ_SZIE);
        }

        /* 消息索引 */
        const obj_idx_t index{0};

        /* 消息数据大小，最长128字节。不支持变长 */
        const obj_size_t data_size{0};

        uint8_t is_server    : 1;
        const uint8_t wr_access    : 1;

        /* 客户端 读取任务状态（用户只能通过fetchStatus方法进行只读访问）*/
        uint8_t read_status  : 3;

        /* 客户端 写入任务状态（用户只能通过fetchStatus方法进行只读访问）**/
        uint8_t write_status : 3;

        uint8_t placeholdler{0};

        FcnCallbackInterface* callback{nullptr};

        static const int MAX_OBJ_SZIE = 0xFF;

        /*
         * 取得子类数据对象。无回调，则子类必须将数据放在第一个成员；有回调，则放在回调对象之后
         */
        inline void* getDataPtr(){
            return ((uint8_t*)this) + sizeof(ServiceObjectBase);
        }
    };

    /*
     * 服务数据对象（Service Object）字典成员
     * 实现了类型安全的数据存储。
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

    struct ServiceObjectDict{
        explicit ServiceObjectDict(obj_idx_t dict_size) : obj_dict(dict_size){
            obj_dict.resize(dict_size);
        }

//        virtual ~ServiceObjectDict()  = default;

        /*默认字段
         * TODO: 版本校验？
         * 1. 可手动校验：使用快照指令
         * 2. 在网络配置阶段，使用专用协议读取*/
//        RtoDictItemNoCb<uint32_t> version;

        utils::vector_s<ServiceObjectBase*> obj_dict;
    };


}
#endif //LIBFCN_V2_DATAOBJECTS_HPP
