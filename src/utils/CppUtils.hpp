//
// Created by sdong on 2019/11/8.
//

#ifndef LIBFCN_CPPUTILS_HPP
#define LIBFCN_CPPUTILS_HPP
#include <cstdint>
#include <memory>
#ifdef SYSTYPE_FULL_OS
#include <vector>
#include <chrono>
#include <cassert>
#include <cstdio>
#endif //SYSTYPE_FULL_OS

#ifdef WIN32
#include <windows.h>	/* WinAPI */

/* Windows sleep in 100ns units */
inline BOOLEAN nanosleep(LONGLONG ns){
    /* Declarations */
    HANDLE timer;	/* Timer handle */
    LARGE_INTEGER li;	/* Time defintion */
    /* Create timer */
    if(!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
        return FALSE;
    /* Set timer properties */
    li.QuadPart = -ns;
    if(!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)){
        CloseHandle(timer);
        return FALSE;
    }
    /* Start & wait for timer */
    WaitForSingleObject(timer, INFINITE);
    /* Clean resources */
    CloseHandle(timer);
    /* Slept without problems */
    return TRUE;
}

#endif

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
        #ifndef WIN32
        timespec ts = double2Timespec(time_d);
        nanosleep(&ts, nullptr);
        #else // WIN32
        nanosleep(time_d * 10e6);
        #endif // WIN32
    }


    #include <stdio.h>
    #include <sys/ioctl.h> // For FIONREAD
    #include <termios.h>
    #include <stdbool.h>

    /**
     * 检测是否有按键按下。不等待回车即可立即返回键盘输入，包括ESC等特殊字符
     * https://stackoverflow.com/questions/421860/capture-characters-from-standard-input-without-waiting-for-enter-to-be-pressed
     * while (!utils::kbhit()) {
     *    utils::perciseSleep(0.05);
     * }
     * c = getchar();
     */
    inline int kbhit(void) {
        static bool initflag = false;
        static const int STDIN = 0;

        if (!initflag) {
            // Use termios to turn off line buffering
            struct termios term;
            tcgetattr(STDIN, &term);
            term.c_lflag &= ~ICANON;
            tcsetattr(STDIN, TCSANOW, &term);
            setbuf(stdin, NULL);
            initflag = true;
        }

        int nbbytes;
        ioctl(STDIN, FIONREAD, &nbbytes);  // 0 is STDIN
        return nbbytes;
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
#ifndef Release
    #define USER_ASSERT(t) if(!(t)){ fprintf(stderr, \
            "!Assert failed: %s at %s:%d\n\r", #t, __FILE__, __LINE__); \
            exit(-1);                                \
            }

    #define USER_IASSERT(t, info) if(!(t)){ fprintf(stderr, \
            "!Assert failed: %s at %s:%d\n\r", #t, __FILE__, __LINE__); \
            fprintf(stderr,  "    ---> reason: %s\n\r",info);                                    \
            exit(-1);                                \
            }
#endif //Release
#else //SYSTYPE_FULL_OS
//TODO: assert
    #define USER_ASSERT(t) while(!(t));
#endif //SYSTYPE_FULL_OS

}

/* support make_unique on C++11 */

#ifndef WIN32
    #if __cplusplus <= 201103L
    namespace std{
        template<typename T, typename... Args>
        std::unique_ptr<T> make_unique(Args&&... args) {
            return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
        }
    }
    #endif
#endif //WIN32

#endif //LIBFCN_CPPUTILS_HPP
