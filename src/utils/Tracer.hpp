//
// Created by sdong on 2019/11/14.
//

#ifndef LIBFCN_TRACER_HPP
#define LIBFCN_TRACER_HPP

#include "LLComDevice.hpp"
#include "vector_s.hpp"
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

    private:
        void batchWrite(const uint8_t *data, uint32_t len);

        bool enable_color;
        Level filter_level {lVerbose};
        vector_s<LLByteDevice*> device;

        char tag[64];

#ifdef SYSTYPE_FULL_OS
        std::mutex update_mutex;
#endif //SYSTYPE_FULL_OS

        uint64_t timestamp_last{0};
    };

}

utils::Tracer* getDefaultTracer();

#ifdef ENABLE_TRACE
    #define LOGV(...) getDefaultTracer()->printf(utils::Tracer::lVerbose, __VA_ARGS__)
    #define LOGI(...) getDefaultTracer()->printf(utils::Tracer::lInfo,    __VA_ARGS__)
    #define LOGD(...) getDefaultTracer()->printf(utils::Tracer::lDebug,   __VA_ARGS__)
    #define LOGW(...) getDefaultTracer()->printf(utils::Tracer::lWarning, __VA_ARGS__)
    #define LOGE(...) getDefaultTracer()->printf(utils::Tracer::lError,   __VA_ARGS__)
    #define LOGF(...) getDefaultTracer()->printf(utils::Tracer::lFatal,   __VA_ARGS__)
#else
    #define LOGI(...) do{}while(0)
    #define LOGV(...) do{}while(0)
    #define LOGD(...) do{}while(0)
    #define LOGW(...) do{}while(0)
    #define LOGE(...) do{}while(0)
    #define LOGF(...) do{}while(0)
#endif

#endif //LIBFCN_TRACER_HPP
