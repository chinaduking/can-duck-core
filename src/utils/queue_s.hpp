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
            return (oldest_idx == newest_idx_next);
        }

        uint32_t size(){
            return calc_size();//  valid_cnt;
        }

        /* oldest value ref */
        T_Val& front(){
            return buffer[oldest_idx];
        }

        /* newest value ref*/
        T_Val& back(){
            USER_ASSERT(!empty());
            return buffer[newest_idx_next];
        }

        void push(T_Val& val){
            push(std::move(val));
        }

        //TODO: release old!!
        void push(T_Val&& val) {
            if (!overwrite_old && is_full()) {
                return;
            }

            buffer[newest_idx_next] = std::move(val);
            newest_idx = newest_idx_next; // hold current new index



            newest_idx_next ++;

            if (newest_idx_next >= capicity) {
                newest_idx_next = 0;
            }

            if (newest_idx_next == oldest_idx){
                oldest_idx ++;
                if(oldest_idx >= capicity){
                    oldest_idx = 0;
                }
            }

            for(int i = 0; i < capicity; i ++){
                LOGD("buffer[%d]=%d", i , buffer[i]);
            }
            LOGD("---> old:%d, new %d", oldest_idx, newest_idx_next);
        }


        //TODO: release old!!
        void pop(){
            if(oldest_idx != newest_idx_next){
                oldest_idx ++;

                if(oldest_idx >= capicity){
                    oldest_idx = 0;
                }
            }

            for(int i = 0; i < capicity; i ++){
                LOGD("buffer[%d]=%d", i , buffer[i]);
            }
            LOGD("---> old:%d, new %d", oldest_idx, newest_idx_next);
        }

        T_Val& operator[](uint32_t index){
            USER_ASSERT(oldest_idx != newest_idx_next);

            uint32_t index_start = oldest_idx;

            index += index_start;

            if(index >= capicity){
                index -= capicity;
            }

            return buffer[index];
        }

    private:
        T_Val* buffer {nullptr};
        uint32_t capicity{0};

        uint32_t oldest_idx{0};
        uint32_t newest_idx_next{0};
        uint32_t newest_idx{0};

        uint32_t size_cnt{0};

        const bool overwrite_old{false};

        bool is_full(){
            /* old = 0, new = capicity */
            if(newest_idx_next > oldest_idx){
                return (oldest_idx == 0 && newest_idx_next == capicity - 1);
            }
            else{
                return (newest_idx_next == oldest_idx - 1);
            }
        }

        uint32_t calc_size(){
            /* 新的在前面 */
            if(newest_idx_next >= oldest_idx){
                return (newest_idx_next - oldest_idx);
            } else{
                /* 老的在前面 */
                return (capicity - oldest_idx + newest_idx_next);;
            }
        }
    };
}

#endif //LIBFCN_QUEUE_S_HPP
