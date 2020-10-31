//
// Created by sdong on 2020/10/29.
//

#ifndef LIBFCN_V2_LOG_HPP
#define LIBFCN_V2_LOG_HPP

#include "utils/Tracer.hpp"
#include "FrameUtils.hpp"


namespace libfcn_v2{
    class Log {
    public:
        static utils::Tracer* getInstance();

    private:
        Log() = default;
        ~Log() = default;

        static utils::Tracer* tracer;
    };
}



#endif //LIBFCN_V2_LOG_HPP
