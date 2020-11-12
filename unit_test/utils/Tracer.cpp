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

    tracer.setFilter(LogLvl::lVerbose);
    tracer.printf(LogLvl::lVerbose, "hello world!!  %d", 2020);
    tracer.printf(LogLvl::lInfo, "hello world!!  %d", 2020);
    tracer.printf(LogLvl::lDebug, "hello world!!");
    tracer.printf(LogLvl::lWarning, "hello world!! %d", 2020);
    tracer.printf(LogLvl::lError, "hello world!!");
    tracer.printf(LogLvl::lFatal, "hello world!!");
}


TEST(Tracer, MACRO){
    FileIODviceWrapper file_wrapper("log.txt");
    getTracer().setFilter(LogLvl::lVerbose);


    LOGV("hello world!!  %d", 2020);
    LOGI("hello world!!  %d", 2020);
    LOGD("hello world!!");
    LOGE("hello world!! %d", 2020);
    LOGW("hello world!!");
    LOGF("hello world!!");
}