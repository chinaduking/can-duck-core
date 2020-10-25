//
// Created by sdong on 2019/11/24.
//

#ifndef LIBFCN_STATICSET_HPP
#define LIBFCN_STATICSET_HPP

#include <cstdint>
//#include <cassert>
#include "CppUtils.hpp"
namespace utils{

    template<class T>
    class vector_s{
    public:
        explicit vector_s(uint32_t capicity_): member_cnt(0), capicity_
        (capicity_) {
            array = new T[capicity_];
            USER_ASSERT(array != nullptr);
        }

        ~vector_s() {
            delete [] array;
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


    private:
        T* array;
        uint32_t member_cnt;
        uint32_t capicity_;
    };
}

#endif //LIBFCN_STATICSET_HPP
