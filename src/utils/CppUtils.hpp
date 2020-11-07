//
// Created by sdong on 2019/11/8.
//

#ifndef LIBFCN_CPPUTILS_HPP
#define LIBFCN_CPPUTILS_HPP
#include <cstdint>

#ifdef SYSTYPE_FULL_OS
#include <vector>
#include <chrono>
#endif //SYSTYPE_FULL_OS

namespace utils{

#ifdef SYSTYPE_FULL_OS
    //return us timestamp..
    inline uint64_t getCurrentTimeUs() {
        using namespace std::chrono;
        return (uint64_t)(duration_cast<microseconds>(
                high_resolution_clock::now().time_since_epoch()
        ).count());
    }

    template <class T>
    bool existInVector(std::vector<T>& list, T key){
        for(auto k : list){
            if(k == key){
                return true;
            }
        }
        return false;
    }

    inline timespec double2Timespec(double time_d){
        timespec t ;
        if(time_d < 0.0f){
            t.tv_sec = 10000;
            t.tv_nsec = 0;
            return t;
        }

        /*tv_nsec = 0 ~ 1.0 - 1e-9*/
        if(time_d > 1.0-1e-8){
            t.tv_sec = (uint64_t) time_d;
        } else{
            t.tv_sec = 0;
        }

        t.tv_nsec = (uint64_t)((time_d - (double)t.tv_sec) * 1e9);

        return t;
    }

    inline void perciseSleep(double time_d){
        timespec ts = double2Timespec(time_d);
        nanosleep(&ts, nullptr);
    }
#endif //SYSTYPE_FULL_OS


    /* 不使用cstring中的memcpy，以减小代码体积。
     * 当数据超过4字节时，一次拷贝4字节，尽可能利用
     * CPU字长。
     * */
    inline void memcpy(void* dest, void* src, uint32_t len){
        uint32_t len_int32 = (len >> 2);

        uint32_t i = 0;

        for(i = 0; i < len_int32; i ++){
            *(((uint32_t*)dest) + i) = *(((uint32_t*)src) + i);
        }


        for(i = len_int32 << 2; i < len; i ++){
            *((uint8_t*)dest + i) = *((uint8_t*)src + i);
        }
    }

    /* 不使用cstring中的memcpy，以减小代码体积。
     * 当数据超过4字节时，一次拷贝4字节，尽可能利用
     * CPU字长。
     * */
    inline void memset(void* dest, int src, uint32_t len){
        uint32_t i = 0;

        for(i = 0; i < len; i ++){
            *(((uint8_t*)dest) + i) = src;
        }
    }

    template<typename T>
    struct is_pointer { static const bool value = false; };

    template<typename T>
    struct is_pointer<T*> { static const bool value = true; };

#ifdef SYSTYPE_FULL_OS
    #define USER_ASSERT(t) assert(t)

#else //SYSTYPE_FULL_OS
//TODO: assert
    #define USER_ASSERT(t) while(!(t));
#endif //SYSTYPE_FULL_OS

}

#endif //LIBFCN_CPPUTILS_HPP
