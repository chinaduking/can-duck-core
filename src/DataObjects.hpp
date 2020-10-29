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

    typedef uint16_t mapped_ptr_t;


    struct ObjectMetaInfo{
        ObjectMetaInfo(obj_idx_t index, obj_size_t data_size)
            : index(index), data_size(data_size){ }

        /* 消息索引 */
        const obj_idx_t  index{0};

        /* 消息数据大小，最长128字节。不支持变长 */
        const obj_size_t data_size{0};

        mapped_ptr_t buffer_offset{0};
    };


    /*
     * 实时数据对象（Real-Time Object）字典成员
     * 实现了类型安全的数据存储。
     * */

    /*
     * 不支持回调的字典项目
     * */
    template <typename T>
    struct ObjectPrototype : public ObjectMetaInfo{
        explicit ObjectPrototype(obj_idx_t index):
                ObjectMetaInfo(index, sizeof(T)){
            utils::memset(&data, 0, sizeof(T));
        }

        void operator<<(T input) { data = input; }
        void operator>>(T &input) { input = data; }

        T data;
    };
#pragma pack(0)


    struct ObjectDictMM{

        ObjectDictMM(obj_idx_t dict_size,
                     ObjectMetaInfo* p_first_obj,
                     void* p_buffer = nullptr)

            :
                p_first_obj(p_first_obj),
                obj_base_offset(dict_size),
                dict_size(dict_size),
                p_buffer(p_buffer){

            obj_base_offset.resize(dict_size);
//            buffer_data_offset.resize(dict_size);
        }


        inline ObjectMetaInfo* getObjBaseByIndex(uint16_t index){
            if(index >= dict_size){
                return nullptr;
            }

            return(ObjectMetaInfo*)(
                    (uint8_t*)p_first_obj + obj_base_offset[index]);
        }


        inline uint8_t* getBufferDataPtr(uint16_t index){
            if(p_buffer == nullptr){
                return nullptr;
            }

            auto rto_base_ptr = getObjBaseByIndex(index);

            if(rto_base_ptr == nullptr){
                return 0;
            }

            return (uint8_t*)p_buffer + rto_base_ptr->buffer_offset;
        }


        inline uint8_t getBufferDataSize(uint16_t index){
            auto rto_base_ptr = getObjBaseByIndex(index);

            if(rto_base_ptr == nullptr){
                return 0;
            }

            return rto_base_ptr->data_size;
        }


        inline uint8_t dictSize(){
            return obj_base_offset.size();
        }

        template<typename Prototype>
        Prototype read(Prototype&& msg){
            USER_ASSERT(p_buffer!= nullptr);

            Prototype res = msg;

            utils::memcpy(&res.data,
                          (uint8_t*)p_buffer +
                                  msg.buffer_offset,
                          sizeof(res.data));

            return res;
        }

        template<typename Prototype>
        void write(Prototype&& msg){
            USER_ASSERT(p_buffer!= nullptr);

            utils::memcpy((uint8_t*)p_buffer +
                                  msg.buffer_offset,
                          &msg.data,
                          sizeof(msg.data));
        }

        /*默认字段
         * TODO: 版本校验？
         * 1. 可手动校验：使用快照指令
         * 2. 在网络配置阶段，使用专用协议读取
         *
         * TODO: 在代码生成器中再添加*/
//        RtoDictItemNoCb<uint32_t> version;

        ObjectMetaInfo* const p_first_obj;

        utils::vector_s<mapped_ptr_t> obj_base_offset;

        uint16_t const dict_size  {0};

        void* const p_buffer{nullptr};

        //TODO:SVO也可以采用这个dict。权限可使用BIT-LUT实现，不新开obj基类
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
                wr_access(wr_access),
                data_size(data_size)


//                is_server(0),
//                read_status(static_cast<uint8_t>
//                            (SvoClientStat::Idle)),
//                write_status(static_cast<uint8_t>
//                             (SvoClientStat::Idle))
                             {
            USER_ASSERT(data_size <= MAX_OBJ_SZIE);
        }

        /* 消息索引 */
        const obj_idx_t index{0};

        /* 消息数据大小，最长64字节。不支持变长 */
        const obj_size_t wr_access : 1;

        const obj_size_t data_size : 7;
//        uint8_t is_server    : 1;
//        const uint8_t wr_access    : 1;

//        /* 客户端 读取任务状态（用户只能通过fetchStatus方法进行只读访问）*/
//        uint8_t read_status  : 3;
//
//        /* 客户端 写入任务状态（用户只能通过fetchStatus方法进行只读访问）**/
//        uint8_t write_status : 3;
//
//        uint8_t placeholdler{0};

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
