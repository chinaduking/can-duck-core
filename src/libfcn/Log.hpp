//
// Created by sdong on 2020/10/29.
//

#ifndef LIBFCN_V2_LOG_HPP
#define LIBFCN_V2_LOG_HPP

#include "Tracer.hpp"
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
    #define LOGI(...) Log::printf(utils::Tracer::Level::lInfo,    __VA_ARGS__)
    #define LOGV(...) Log::printf(utils::Tracer::Level::VERBOSE, __VA_ARGS__)
    #define LOGD(...) Log::printf(utils::Tracer::Level::lDebug,   __VA_ARGS__)
    #define LOGW(...) Log::printf(utils::Tracer::Level::lWarning, __VA_ARGS__)
    #define LOGE(...) Log::printf(utils::Tracer::Level::lError,   __VA_ARGS__)
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



#endif //LIBFCN_V2_LOG_HPP
