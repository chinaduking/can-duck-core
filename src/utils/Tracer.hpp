//
// Created by sdong on 2019/11/14.
//

#ifndef LIBFCN_TRACER_HPP
#define LIBFCN_TRACER_HPP

#include "LLComDevice.hpp"
#include "vector_s.hpp"

#ifdef SYSTYPE_FULL_OS
#include <mutex>
#include "HostIODeviceWrapper.hpp"

#endif //SYSTYPE_FULL_OS

namespace utils{
    class Tracer{
    public:
        enum class Level : uint8_t{
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
        int print(Level level, char* fmt, ...);

    private:
        void batchWrite(const uint8_t *data, uint32_t len);

        bool enable_color;
        Level filter_level {Level::lVerbose};
        vector_s<LLByteDevice*> device;

        char tag[64];

#ifdef SYSTYPE_FULL_OS
        std::mutex update_mutex;
        StdoutIODviceWrapper stdio_wrapper;
#endif //SYSTYPE_FULL_OS

        uint64_t timestamp_last{0};
    };

}

#endif //LIBFCN_TRACER_HPP
