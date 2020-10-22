//
// Created by sdong on 2019/11/14.
//

#ifndef LIBFCN_TRACER_HPP
#define LIBFCN_TRACER_HPP

#include "../LLComDevice.hpp"
#include "vector_s.hpp"

#ifdef SYSTYPE_FULL_OS
#include <mutex>
#include "HostIODeviceWrapper.hpp"

#endif //SYSTYPE_FULL_OS

namespace utils{
    class Tracer{
    public:
        enum Level{
            NONE = 0,
            INFO,
            VERBOSE,
            WARNING,
            ERROR,
            FATAL
        };

        Tracer(bool enable_color=false);
        ~Tracer() = default;

        void setFilter(Level level);
        void addByteIODeviece(LLByteDevice* device);
        void setTag(char* tag);

        /**/
        void print(Level level, char* fmt, ...);

    private:
        void batchWrite(const uint8_t *data, uint32_t len);

        bool enable_color;
        Level filter_level {WARNING};
        vector_s<LLByteDevice*> device;

        char tag[64];

#ifdef SYSTYPE_FULL_OS
        std::mutex update_mutex;
        StdoutIODviceWrapper stdio_wrapper;
#endif //SYSTYPE_FULL_OS


    };

}

#endif //LIBFCN_TRACER_HPP
