//
// Created by sdong on 2020/10/21.
//

#include "libfcn/TestUtils.hpp"
#include "utils/Tracer.hpp"
#include "utils/HostIODeviceWrapper.hpp"

using namespace utils;

TEST(Tracer, basic){
    Tracer tracer(true);

//    StdoutIODviceWrapper stdio_wrapper;
    FileIODviceWrapper file_wrapper("log.txt");

//    tracer.addByteIODeviece(&stdio_wrapper);
    tracer.addByteIODeviece(&file_wrapper);

    tracer.setFilter(Tracer::Level::lVerbose);
    tracer.print(Tracer::Level::lVerbose, "hello world!!  %d", 2020);
    tracer.print(Tracer::Level::lInfo, "hello world!!  %d", 2020);
    tracer.print(Tracer::Level::lDebug, "hello world!!");
    tracer.print(Tracer::Level::lWarning, "hello world!! %d", 2020);
    tracer.print(Tracer::Level::lError, "hello world!!");
    tracer.print(Tracer::Level::lFatal, "hello world!!");
}