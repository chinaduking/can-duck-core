//
// Created by sdong on 2019/11/24.
//

#ifndef LIBFCN_STATICSET_HPP
#define LIBFCN_STATICSET_HPP

#include <cstdint>
//#include <cassert>
#include "CppUtils.hpp"
#include "ObjPool.hpp"
namespace utils{

    template<typename T, typename Allocator=DefaultAllocator>
    class vector_s{
    public:
        explicit vector_s(uint32_t capicity_): member_cnt(0), capicity_
        (capicity_) {
            array = new T[capicity_];
//          array = Allocator::allocate(capicity_*sizeof(T));
            USER_ASSERT(array != nullptr);
        }

        ~vector_s() {

            delete [] array;

//            Allocator::deallocate(array);
        }

        uint32_t push_back(T x){
            USER_ASSERT(member_cnt < capicity_);

            if(member_cnt < capicity_){
                array[member_cnt++] = x;
            }

            return member_cnt - 1;
        }

        T pop(){
            if(member_cnt > 0){
                return array[--member_cnt];
            }
            return array[0];
        }

        bool empty(){
            return member_cnt == 0;
        }

        uint32_t size(){
            return member_cnt;
        }

        uint32_t capicity(){
            return capicity_;
        }

        T& operator[](uint32_t x){
            USER_ASSERT(x < member_cnt);

            return array[x];
        }

        void resize(uint32_t size){
            if(size > capicity_){
                size = capicity_;
            }
            member_cnt = size;
        }

        /* 迭代器方法，支持range-based for loop */
        class iterator{
        public:
            explicit iterator(const vector_s* static_set) : static_set(static_set),
                                                            index(0){}
            ~iterator() = default;

            iterator operator++() {
                index++;
                return *this;
            }

            bool operator!=(const iterator & other) const {
                return index != static_set->member_cnt;
            }

            T& operator*() const {
                return static_set->array[index];
            }

        private:
            const vector_s* static_set;
            uint64_t index;

        };

        iterator begin() const { return iterator(this); }
        iterator end() const { return iterator(this); }


        void remove_if(bool(*matched)(T& data)){
            uint32_t matched_idx = member_cnt;

            for(uint32_t i = 0; i < member_cnt; i++){
                if((*matched)(array[i])){
                    matched_idx = i;
                }
            }

            remove(matched_idx);
        }


        void remove(uint32_t index){
            if(index >= member_cnt){
                return;
            }

            /* 最后一个，直接-1 */
            if(index == member_cnt - 1){
                member_cnt --;
                ~array[index]();
                return;
            }

            /* 中间的，后面递推 */
            for(uint32_t i = index; i < member_cnt; i++){
            	~array[index]();
                array[i] = array[i+1];
            }
            member_cnt --;

        }

    private:
        T* array;
        uint32_t member_cnt;
        uint32_t capicity_;
    };
}

#endif //LIBFCN_STATICSET_HPP
