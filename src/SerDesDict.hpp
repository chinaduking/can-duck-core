//
// Created by sdong on 2020/10/26.
//

#ifndef can_duck_SERDESDICT_HPP
#define can_duck_SERDESDICT_HPP

#include <cstdint>
#include "CppUtils.hpp"
#include "Vector.hpp"
#include "BitLUT8.hpp"


namespace can_duck {

    /* 数据长度标志位为无符号8位整形，最大255
     **/
    typedef uint8_t obj_idx_t;
    typedef uint8_t obj_size_t;
    typedef uint16_t mapped_ptr_t;

#pragma pack(2)
    struct hDictItem{
        hDictItem(obj_idx_t index, obj_size_t data_size)
            : index(index), data_size(data_size){ }

        /* 消息索引 */
        const obj_idx_t  index{0};

        /* 消息数据大小，最长128字节。不支持变长 */
        const obj_size_t data_size{0};

        /* 缓冲区中数据的偏移量 */
        mapped_ptr_t buffer_offset{0};

        inline void* getDataPtr(){
            return ((uint8_t*)this) + sizeof(hDictItem);
        }
    };
#pragma pack(0)


#pragma pack(2)
    template <typename T>
    struct DictItem : public hDictItem{
        explicit DictItem(obj_idx_t index):
                hDictItem(index, sizeof(T)){
            emlib::memset(&data, 0, sizeof(T));
        }

        /* 对字典中数据进行复制同时赋值（对外具有原子性） */
        inline DictItem operator () (T&& value){
            DictItem<T> result(this->index);
            result.data = std::move(value);
            return result;
        }

        /* 对字典中数据进行复制同时赋值（对外具有原子性） */
        inline DictItem operator () (T& value){
            DictItem<T> result(this->index);
            result.data = value;
            return result;
        }

        inline void operator<<(T value) { data = value; }
        inline void operator>>(T &value) { value = data; }

        T data;
    };
#pragma pack(0)

#define DICT_T(prototype) decltype(prototype.data)


    struct SerDesDict{

        SerDesDict(obj_idx_t dict_size,
                   hDictItem* p_first_obj):
                p_first_val(p_first_obj),
                val_offset_key_table(dict_size) {
            val_offset_key_table.resize(dict_size);
        }


        inline hDictItem* getObjBaseByIndex(uint16_t index){
            USER_ASSERT(index < val_offset_key_table.size());

            return(hDictItem*)(
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
        inline Prototype deserialize(Prototype&& msg, void* p_buffer){
            USER_ASSERT(p_buffer!= nullptr);

            Prototype res = msg;

            emlib::memcpy(&res.data,
                          (uint8_t*)p_buffer + msg.buffer_offset,
                          sizeof(res.data));

            return res;
        }

        inline bool deserialize(int16_t index, uint8_t* data, void* p_buffer){
            if(index >= dictSize() || p_buffer == nullptr || data == nullptr){
                return false;
            }
            auto obj = getObjBaseByIndex(index);
            emlib::memcpy((uint8_t*)p_buffer + obj->buffer_offset,
                          data,
                          obj->data_size);
            return true;
        }

//        template<typename Prototype>
        inline void handleSerialize(hDictItem& msg, void* p_buffer){
            USER_ASSERT(p_buffer!= nullptr);

            emlib::memcpy((uint8_t*)p_buffer + msg.buffer_offset,
                          msg.getDataPtr(),
                          msg.data_size);
        }

        template<typename Prototype>
        inline void serialize(Prototype&& msg, void* p_buffer){
            USER_ASSERT(p_buffer!= nullptr);

            emlib::memcpy((uint8_t*)p_buffer + msg.buffer_offset,
                          &msg.data,
                          sizeof(msg.data));
        }


        virtual void* createBuffer() = 0;


        hDictItem* const p_first_val;
        emlib::Vector<mapped_ptr_t> val_offset_key_table;
    };

    /*非阻塞式任务的回调函数
     * TODO: custom allocator*/
    typedef void  (*TransferCallbackFunc)(void* obj_ptr, uint8_t ev_code, void*
    msg);

    struct TransferCallback_t {
        TransferCallback_t() = default;
        TransferCallback_t(void* obj_ptr, TransferCallbackFunc callback_func):
                obj_ptr(obj_ptr), callback_func(callback_func){}

        void* obj_ptr {nullptr};
        TransferCallbackFunc callback_func {nullptr};
    };


}

#endif //can_duck_SERDESDICT_HPP
