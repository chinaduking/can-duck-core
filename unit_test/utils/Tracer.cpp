//
// Created by sdong on 2020/10/21.
//

#include "TestUtils.hpp"
#include "utils/Tracer.hpp"
#include "utils/HostIODeviceWrapper.hpp"

using namespace utils;

TEST(Tracer, basic){
    Tracer tracer(true);

//    StdoutIODviceWrapper stdio_wrapper;
    FileIODviceWrapper file_wrapper("log.txt");

//    tracer.addByteIODeviece(&stdio_wrapper);
    tracer.addByteIODeviece(&file_wrapper);

    tracer.setFilter(Tracer::INFO);
    tracer.print(Tracer::INFO, "hello world!!");
    tracer.print(Tracer::VERBOSE, "hello world!!");
    tracer.print(Tracer::WARNING, "hello world!!");
    tracer.print(Tracer::ERROR, "hello world!!");
    tracer.print(Tracer::FATAL, "hello world!!");
}