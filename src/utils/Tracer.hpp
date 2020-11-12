//
// Created by sdong on 2019/11/14.
//

#ifndef LIBFCN_TRACER_HPP
#define LIBFCN_TRACER_HPP

#include "LLComDevice.hpp"
#include "Vector.hpp"
#include "RingBuf.hpp"
#include <string>

namespace utils{

    /* 上位机总是使能log输出功能，下位机发布模式不使能log */
    #ifndef SYSTYPE_FULL_OS
        #ifdef DEBUG
            #define ENABLE_TRACE
        #endif //DEBUG
    #else//SYSTYPE_FULL_OS
        #define ENABLE_TRACE
    #endif //SYSTYPE_FULL_OS

    #ifdef ENABLE_TRACE
//        #warning "----> trace is enable!"
    #endif //ENABLE_TRACE


    class Tracer{
    public:
        static const int FMT_BUF_SZ = 512;

#ifdef SYSTYPE_FULL_OS
        static const int FMT_BUF_QUEUE_SZ = 32;
#else
        static const int FMT_BUF_QUEUE_SZ = 8;
#endif

        //TODO: no enum class !
        enum Level : uint8_t{
            lNone = 0,
            lVerbose,
            lInfo,
            lDebug,
            lWarning,
            lError,
            lFatal
        };

        Tracer(bool enable_color=false);
        ~Tracer() = default;

        void setFilter(Level level);
        void addByteIODeviece(LLByteDevice* device);
        void setTag(char* tag);

        /**/
        int vprintf(Level level, char *format, va_list arg_ptr);
        int printf(Level level, char* fmt, ...);

        struct FmtBuf{
            uint8_t  level{0};
            uint16_t idx {0};

            uint8_t  buf  [FMT_BUF_SZ];
            uint32_t size_limit{ FMT_BUF_SZ };

            uint8_t* fmt_ptr{0};
            uint32_t fmt_len{0};
        };
        //TODO: zero copy format to buffer use external method
        FmtBuf* fmtBufAlloc(Level level);
        void fmtBufPrint(FmtBuf* buf);

    private:
        void batchWrite(const uint8_t *data, uint32_t len);

        bool enable_color;
        Level filter_level {lVerbose};
        Vector<LLByteDevice*> device;

        char tag[64];

#ifdef ENABLE_TRACE
        ObjPool<FmtBuf, FMT_BUF_QUEUE_SZ> frm_buf_poll;
        utils::RingBuf<FmtBuf*> frm_buf_queue;
#endif

#ifdef SYSTYPE_FULL_OS
        std::mutex update_mutex;
#endif //SYSTYPE_FULL_OS

        uint64_t timestamp_last{0};
    };
}

utils::Tracer& getTracer();

typedef utils::Tracer::Level LogLvl;

#ifdef ENABLE_TRACE
    #define LOGV(...) getTracer().printf(LogLvl::lVerbose, __VA_ARGS__)
    #define LOGI(...) getTracer().printf(LogLvl::lInfo,    __VA_ARGS__)
    #define LOGD(...) getTracer().printf(LogLvl::lDebug,   __VA_ARGS__)
    #define LOGW(...) getTracer().printf(LogLvl::lWarning, __VA_ARGS__)
    #define LOGE(...) getTracer().printf(LogLvl::lError,   __VA_ARGS__)
    #define LOGF(...) getTracer().printf(LogLvl::lFatal,   __VA_ARGS__)
#else
    #define LOGI(...) do{}while(0)
    #define LOGV(...) do{}while(0)
    #define LOGD(...) do{}while(0)
    #define LOGW(...) do{}while(0)
    #define LOGE(...) do{}while(0)
    #define LOGF(...) do{}while(0)
#endif

#endif //LIBFCN_TRACER_HPP
