//
// Created by sdong on 2020/10/29.
//

#ifndef LIBFCN_V2_TRACERSINGLETON_HPP
#define LIBFCN_V2_TRACERSINGLETON_HPP

#include "utils/Tracer.hpp"
#include "FrameUtils.hpp"


namespace libfcn_v2{
    class TracerSingleton {
    public:
        static utils::Tracer* getInstance();

    private:
        TracerSingleton() = default;
        ~TracerSingleton() = default;

        static utils::Tracer* tracer;
    };
}



#endif //LIBFCN_V2_TRACERSINGLETON_HPP
