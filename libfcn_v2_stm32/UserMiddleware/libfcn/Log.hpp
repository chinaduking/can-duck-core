//
// Created by sdong on 2020/10/29.
//

#ifndef LIBFCN_V2_LOG_HPP
#define LIBFCN_V2_LOG_HPP

#ifdef SYSTYPE_FULL_OS
#include "utils/Tracer.hpp"
#include "DataLinkLayer.hpp"
#include "DefaultAllocate.h"

namespace libfcn_v2{
    using LogLevel = utils::Tracer::Level;

    class Log {
    public:
        static utils::Tracer* getInstance();

        static int printf(utils::Tracer::Level level,
                           char *format,
                           ... );

        static void setLevel(LogLevel level);

    private:
        Log() = default;
        ~Log() = default;

        static utils::Tracer* tracer;
    };

    std::string Frame2Log(DataLinkFrame& frame);

    std::string Frame2LogCompact(DataLinkFrame& frame);

#ifdef ENABLE_LOG
    #define LOGI(...) Log::printf(utils::Tracer::Level::INFO,    __VA_ARGS__)
    #define LOGV(...) Log::printf(utils::Tracer::Level::VERBOSE, __VA_ARGS__)
    #define LOGD(...) Log::printf(utils::Tracer::Level::DEBUG,   __VA_ARGS__)
    #define LOGW(...) Log::printf(utils::Tracer::Level::WARNING, __VA_ARGS__)
    #define LOGE(...) Log::printf(utils::Tracer::Level::ERROR,   __VA_ARGS__)
    #define LOGF(...) Log::printf(utils::Tracer::Level::FATAL,   __VA_ARGS__)
#else
    #define LOGI(...) do{}while(0)
    #define LOGV(...) do{}while(0)
    #define LOGD(...) do{}while(0)
    #define LOGW(...) do{}while(0)
    #define LOGE(...) do{}while(0)
    #define LOGF(...) do{}while(0)
#endif

}

#else //SYSTYPE_FULL_OS
#define LOGI(...) do{}while(0)
#define LOGV(...) do{}while(0)
#define LOGD(...) do{}while(0)
#define LOGW(...) do{}while(0)
#define LOGE(...) do{}while(0)
#define LOGF(...) do{}while(0)
#endif //SYSTYPE_FULL_OS


#endif //LIBFCN_V2_LOG_HPP
