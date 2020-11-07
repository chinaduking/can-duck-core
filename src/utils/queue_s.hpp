//
// Created by sdong on 2020/11/7.
//

#ifndef LIBFCN_QUEUE_S_HPP
#define LIBFCN_QUEUE_S_HPP

#include <cstdint>
#include "CppUtils.hpp"
#include "Tracer.hpp"

namespace utils{

    template<typename T_Val>
    class queue_s{
    public:
        queue_s(uint32_t capicity, bool overwrite_old=true):
                capicity(capicity),
                overwrite_old(overwrite_old){
            buffer = new T_Val[capicity];
        }
        ~queue_s(){
            delete [] buffer;
        }

        bool empty(){
            return size_cnt == 0;
//            return (oldest_idx == newest_idx_next);
        }

        uint32_t size(){
            return size_cnt;
        }

        /* oldest value ref */
        T_Val& front(){
//            USER_ASSERT(!empty());
            return buffer[oldest_idx];
        }

        /* newest value ref*/
        T_Val& back(){
            USER_ASSERT(!empty());
            return buffer[newest_idx];
        }

        void push(T_Val& val){
            push(std::move(val));
        }

        //TODO: release old!!
        void push(T_Val&& val) {
            if (!overwrite_old && size_cnt == capicity) {
                return;
            }

            size_cnt ++;

            auto max_index = capicity - 1;

            if(newest_idx == max_index && oldest_idx == 0){
                oldest_idx ++;
                size_cnt --;
            }
            if(newest_idx == oldest_idx - 1){
                oldest_idx ++;
                size_cnt --;
            }

            if(oldest_idx > max_index){
                oldest_idx = 0;
            }

            buffer[newest_idx_next] = std::move(val);

            newest_idx = newest_idx_next; // hold current new index

            newest_idx_next ++;

            if (newest_idx_next > max_index) {
                newest_idx_next = 0;
            }


//            for(int i = 0; i < capicity; i ++){
//                LOGD("buffer[%d]=%d", i , buffer[i]);
//            }
//            LOGD("---> old:%d, new %d", oldest_idx, newest_idx_next);
        }


        //TODO: release old!!
        void pop(){
            if(empty()){
                return;
            }

            size_cnt --;

            oldest_idx ++;

            if(oldest_idx >= capicity){
                oldest_idx = 0;
            }

//            for(int i = 0; i < capicity; i ++){
//                LOGD("buffer[%d]=%d", i , buffer[i]);
//            }
//            LOGD("---> old:%d, new %d", oldest_idx, newest_idx_next);
        }

        T_Val& operator[](uint32_t index){
            USER_ASSERT(index < size_cnt);

            uint32_t index_start = oldest_idx;

            index += index_start;

            if(index >= capicity){
                index -= capicity;
            }

            return buffer[index];
        }

    private:
        T_Val* buffer {nullptr};
        const uint32_t capicity{0};

        uint32_t oldest_idx{0};
        uint32_t newest_idx_next{0};
        uint32_t newest_idx{0};

        uint32_t size_cnt{0};

        const bool overwrite_old{false};
    };
}

#endif //LIBFCN_QUEUE_S_HPP
