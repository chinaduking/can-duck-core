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

    tracer.setFilter(Tracer::lVerbose);
    tracer.printf(Tracer::lVerbose, "hello world!!  %d", 2020);
    tracer.printf(Tracer::lInfo, "hello world!!  %d", 2020);
    tracer.printf(Tracer::lDebug, "hello world!!");
    tracer.printf(Tracer::lWarning, "hello world!! %d", 2020);
    tracer.printf(Tracer::lError, "hello world!!");
    tracer.printf(Tracer::lFatal, "hello world!!");
}


TEST(Tracer, MACRO){
    FileIODviceWrapper file_wrapper("log.txt");
    getDefaultTracer()->setFilter(Tracer::lVerbose);


    LOGV("hello world!!  %d", 2020);
    LOGI("hello world!!  %d", 2020);
    LOGD("hello world!!");
    LOGE("hello world!! %d", 2020);
    LOGW("hello world!!");
    LOGF("hello world!!");
}