//
// Created by sdong on 2019/11/24.
//

#ifndef LIBFCN_ARRAY_HPP
#define LIBFCN_ARRAY_HPP
#include <cstdint>
#include <cassert>
namespace utils{
    template<class T, uint32_t N>
    class Array{
    public:
        Array() = default;

        ~Array() = default;

        T array[N];

        T& operator[](uint32_t x){
            assert(x < N);
            return array[x];
        }

        uint32_t size(){
            return N;
        }
    };
}

#endif //LIBFCN_ARRAY_HPP
