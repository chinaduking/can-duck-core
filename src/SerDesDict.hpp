//
// Created by sdong on 2020/10/26.
//

#ifndef LIBFCN_V2_SERDESDICT_HPP
#define LIBFCN_V2_SERDESDICT_HPP

#include <cstdint>
#include "utils/CppUtils.hpp"
#include "utils/vector_s.hpp"
#include "utils/BitLUT8.hpp"


namespace libfcn_v2 {

    /* 数据长度标志位为无符号8位整形，最大255
     **/
    typedef uint8_t obj_idx_t;
    typedef uint8_t obj_size_t;
    typedef uint16_t mapped_ptr_t;

#pragma pack(2)
    struct SerDesDictValHandle{
        SerDesDictValHandle(obj_idx_t index, obj_size_t data_size)
            : index(index), data_size(data_size){ }

        /* 消息索引 */
        const obj_idx_t  index{0};

        /* 消息数据大小，最长128字节。不支持变长 */
        const obj_size_t data_size{0};

        /* 缓冲区中数据的偏移量 */
        mapped_ptr_t buffer_offset{0};

        inline void* getDataPtr(){
            return ((uint8_t*)this) + sizeof(SerDesDictValHandle);
        }
    };
#pragma pack(0)


#pragma pack(2)
    template <typename T>
    struct SerDesDictVal : public SerDesDictValHandle{
        explicit SerDesDictVal(obj_idx_t index):
                SerDesDictValHandle(index, sizeof(T)){
            utils::memset(&data, 0, sizeof(T));
        }

        void operator<<(T input) { data = input; }
        void operator>>(T &input) { input = data; }

        T data;
    };
#pragma pack(0)


    struct SerDesDict{

        SerDesDict(obj_idx_t dict_size,
                   SerDesDictValHandle* p_first_obj):
                p_first_val(p_first_obj),
                val_offset_key_table(dict_size) {
            val_offset_key_table.resize(dict_size);
        }


        inline SerDesDictValHandle* getObjBaseByIndex(uint16_t index){
            USER_ASSERT(index < val_offset_key_table.size());

            return(SerDesDictValHandle*)(
                    (uint8_t*)p_first_val + val_offset_key_table[index]);
        }

        inline uint32_t getBufferDataOffest(uint16_t index){
            return getObjBaseByIndex(index)->buffer_offset;
        }


        inline uint8_t getBufferDataSize(uint16_t index){
            return getObjBaseByIndex(index)->data_size;
        }

        inline uint8_t dictSize(){
            return val_offset_key_table.size();
        }

        template<typename Prototype>
        Prototype deserialize(Prototype&& msg, void* p_buffer){
            USER_ASSERT(p_buffer!= nullptr);

            Prototype res = msg;

            utils::memcpy(&res.data,
                          (uint8_t*)p_buffer + msg.buffer_offset,
                          sizeof(res.data));

            return res;
        }

        template<typename Prototype>
        void serialize(Prototype&& msg, void* p_buffer){
            USER_ASSERT(p_buffer!= nullptr);

            utils::memcpy((uint8_t*)p_buffer + msg.buffer_offset,
                          &msg.data,
                          sizeof(msg.data));
        }

        virtual void* createBuffer() = 0;


        SerDesDictValHandle* const p_first_val;
        utils::vector_s<mapped_ptr_t> val_offset_key_table;
    };

    /*非阻塞式任务的回调函数
     * TODO: custom allocator*/
    struct FcnCallbackInterface {
        virtual void callback(void *data, uint8_t ev_code) = 0;
    };


}

#endif //LIBFCN_V2_SERDESDICT_HPP
