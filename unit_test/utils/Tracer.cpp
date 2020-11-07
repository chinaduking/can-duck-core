//
// Created by sdong on 2020/10/21.
//

#include "libfcn/TestUtils.hpp"
#include "utils/Tracer.hpp"
#include "utils/os_only/HostIODeviceWrapper.hpp"

using namespace utils;

TEST(Tracer, basic){
    Tracer tracer(true);

//    StdoutIODviceWrapper stdio_wrapper;
    FileIODviceWrapper file_wrapper("log.txt");

//    tracer.addByteIODeviece(&stdio_wrapper);
    tracer.addByteIODeviece(&file_wrapper);

    tracer.setFilter(Tracer::Level::lVerbose);
    tracer.printf(Tracer::Level::lVerbose, "hello world!!  %d", 2020);
    tracer.printf(Tracer::Level::lInfo, "hello world!!  %d", 2020);
    tracer.printf(Tracer::Level::lDebug, "hello world!!");
    tracer.printf(Tracer::Level::lWarning, "hello world!! %d", 2020);
    tracer.printf(Tracer::Level::lError, "hello world!!");
    tracer.printf(Tracer::Level::lFatal, "hello world!!");
}


TEST(Tracer, MACRO){
    FileIODviceWrapper file_wrapper("log.txt");
    getDefaultTracer()->setFilter(Tracer::Level::lVerbose);


    LOGV("hello world!!  %d", 2020);
    LOGI("hello world!!  %d", 2020);
    LOGD("hello world!!");
    LOGE("hello world!! %d", 2020);
    LOGW("hello world!!");
    LOGF("hello world!!");
}