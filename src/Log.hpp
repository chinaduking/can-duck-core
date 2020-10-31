//
// Created by sdong on 2020/10/29.
//

#ifndef LIBFCN_V2_LOG_HPP
#define LIBFCN_V2_LOG_HPP

#include "utils/Tracer.hpp"
#include "DataLinkLayer.hpp"


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

    #define LOGI(...) Log::printf(utils::Tracer::Level::INFO,    __VA_ARGS__)
    #define LOGV(...) Log::printf(utils::Tracer::Level::VERBOSE, __VA_ARGS__)
    #define LOGW(...) Log::printf(utils::Tracer::Level::WARNING, __VA_ARGS__)
    #define LOGE(...) Log::printf(utils::Tracer::Level::ERROR,   __VA_ARGS__)
    #define LOGF(...) Log::printf(utils::Tracer::Level::FATAL,   __VA_ARGS__)
}



#endif //LIBFCN_V2_LOG_HPP
