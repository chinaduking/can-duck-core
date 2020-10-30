//
// Created by sdong on 2020/10/26.
//

#ifndef LIBFCN_V2_DATAOBJECTS_HPP
#define LIBFCN_V2_DATAOBJECTS_HPP

#include <cstdint>
#include "utils/CppUtils.hpp"
#include "utils/vector_s.hpp"
#include "utils/BitLUT8.hpp"


namespace libfcn_v2 {

    /* 数据长度标志位为无符号8位整形，最大255
 * */
    typedef uint8_t obj_idx_t;
    typedef uint8_t obj_size_t;

    typedef uint16_t rto_timestamp_t;

    /*非阻塞式任务的回调函数
     * TODO: custom allocator*/
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

        inline void* getDataPtr(){
            return ((uint8_t*)this) + sizeof(ObjectMetaInfo);
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
                obj_base_offset(dict_size)
//                ,dict_size(dict_size)
//                ,p_buffer(p_buffer)
                {

            obj_base_offset.resize(dict_size);
//            buffer_data_offset.resize(dict_size);
        }


        inline ObjectMetaInfo* getObjBaseByIndex(uint16_t index){
            USER_ASSERT(index < obj_base_offset.size());

            return(ObjectMetaInfo*)(
                    (uint8_t*)p_first_obj + obj_base_offset[index]);
        }

        inline uint32_t getBufferDataOffest(uint16_t index){
            return getObjBaseByIndex(index)->buffer_offset;
        }


        inline uint8_t getBufferDataSize(uint16_t index){
            return getObjBaseByIndex(index)->data_size;
        }

        inline uint8_t dictSize(){
            return obj_base_offset.size();
        }

        template<typename Prototype>
        Prototype read(Prototype&& msg, void* p_buffer){
            USER_ASSERT(p_buffer!= nullptr);

            Prototype res = msg;

            utils::memcpy(&res.data,
                          (uint8_t*)p_buffer + msg.buffer_offset,
                          sizeof(res.data));

            return res;
        }

        template<typename Prototype>
        void write(Prototype&& msg, void* p_buffer){
            USER_ASSERT(p_buffer!= nullptr);

            utils::memcpy((uint8_t*)p_buffer + msg.buffer_offset,
                          &msg.data,
                          sizeof(msg.data));
        }


        virtual void* createBuffer() = 0;
//        inline void bindBuffer(void* buffer){
//            p_buffer = buffer;
//        }

        /*默认字段
         * TODO: 版本校验？
         * 1. 可手动校验：使用快照指令
         * 2. 在网络配置阶段，使用专用协议读取
         *
         * TODO: 在代码生成器中再添加*/
//        RtoDictItemNoCb<uint32_t> version;

        ObjectMetaInfo* const p_first_obj;

        utils::vector_s<mapped_ptr_t> obj_base_offset;

//        void* p_buffer{nullptr};

        //TODO:SVO也可以采用这个dict。权限可使用BIT-LUT实现，不新开obj基类
    };

}

#endif //LIBFCN_V2_DATAOBJECTS_HPP
