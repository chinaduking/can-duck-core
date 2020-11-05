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

    tracer.setFilter(Tracer::Level::VERBOSE);
    tracer.print(Tracer::Level::VERBOSE, "hello world!!  %d", 2020);
    tracer.print(Tracer::Level::INFO, "hello world!!  %d", 2020);
    tracer.print(Tracer::Level::DEBUG, "hello world!!");
    tracer.print(Tracer::Level::WARNING, "hello world!! %d", 2020);
    tracer.print(Tracer::Level::ERROR, "hello world!!");
    tracer.print(Tracer::Level::FATAL, "hello world!!");
}