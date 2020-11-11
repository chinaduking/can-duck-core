//
// Created by sdong on 2019/11/14.
//

#include "Tracer.hpp"
#include <cstdio>
#include <cstdarg>
#include <cstring>


#ifdef SYSTYPE_FULL_OS
    /* 如果是OS，包含iostream，以便调用cout<<endl进行跨平台的强制终端输出。*/
    #include <iostream>

    /* 默认可通过stdout进行输出*/
    #include "utils/os_only/HostIODeviceWrapper.hpp"
    utils::StdoutIODviceWrapper stdio_wrapper;

    /* 互斥锁 */
    #include <mutex>
    #define MUTEX_LOCKGUARD std::lock_guard<std::mutex> lk(update_mutex)
#else
	#define MUTEX_LOCKGUARD
#endif //SYSTYPE_FULL_OS

/* 是否输出操作码的语义 */
#define OP_CODE_DECODE


#define TRACE_BUFFER_SIZE 1024
#define MAX_BINDING_OUTPUT_DEVICE 3

using namespace utils;


static char* level_name[] = {
        (char*) "N",
        (char*) "V",
        (char*) "I",
        (char*) ">> D",
        (char*) ">>  !W",
        (char*) ">> !!E",
        (char*) ">>!!!F"
};

#define ANSI_COLOR_RED     (char*) "\x1b[31m"
#define ANSI_COLOR_GREEN   (char*) "\x1b[32m"
#define ANSI_COLOR_YELLOW  (char*) "\x1b[33m"
#define ANSI_COLOR_BLUE    (char*) "\x1b[34m"
#define ANSI_COLOR_MAGENTA (char*) "\x1b[35m"
#define ANSI_COLOR_CYAN    (char*) "\x1b[36m"
#define ANSI_COLOR_RESET   (char*) "\x1b[0m "

static char* level_color[] = {
        ANSI_COLOR_RESET,       //NONE
        ANSI_COLOR_RESET,       //VERBOSE
        ANSI_COLOR_GREEN,       //INFO
        ANSI_COLOR_CYAN,        //DEBUG
        ANSI_COLOR_YELLOW,      //WARNING
        ANSI_COLOR_RED,         //ERROR
        ANSI_COLOR_MAGENTA      //FATAL
};

Tracer* defaultTracer = nullptr;

Tracer* getDefaultTracer(){
    if(defaultTracer == nullptr){
        defaultTracer = new Tracer(true);
    }

    return defaultTracer;
}

Tracer::Tracer(bool enable_color)
    : enable_color(enable_color),
      device(MAX_BINDING_OUTPUT_DEVICE)
{
    tag[0] = 0;

#ifdef SYSTYPE_FULL_OS
    /* 默认可通过stdout进行输出*/
    addByteIODeviece(&stdio_wrapper);

    timestamp_last = getCurrentTimeUs();
#endif //SYSTYPE_FULL_OS
}

void Tracer::setFilter(Level level){
    MUTEX_LOCKGUARD;

    if(level <= lFatal){
        filter_level = level;
    }
}

void Tracer::addByteIODeviece(LLByteDevice* device){
    MUTEX_LOCKGUARD;
    if(device != nullptr){
        this->device.push_back(device);
    }
}


void Tracer::setTag(char* tag){
    MUTEX_LOCKGUARD;

    //TODO: str handle!
    strncpy(this->tag, tag, 64);
    this->tag[63] = 0;
}


void Tracer::batchWrite(const uint8_t *data, uint32_t len) {
    for(int i = 0; i < device.size(); i ++){
        device[i]->write(data, len);
    }
}

char trace_buffer[TRACE_BUFFER_SIZE];

int Tracer::vprintf(Level level, char *format,  va_list arg_ptr) {
    MUTEX_LOCKGUARD;
    int ret = 0;

#ifdef ENABLE_TRACE
    if(filter_level == lNone || device.size() == 0){
        return 0;
    }

    if(level < filter_level || level > lFatal){
        return 0;
    }

    char* str_tmp;
#ifdef SYSTYPE_FULL_OS
    uint64_t timestamp = getCurrentTimeUs();
#else
    uint64_t timestamp = 0;
#endif
    /* Color */
    static int color_str_len = strlen(ANSI_COLOR_RED) + 1;

    if(enable_color){
        str_tmp = level_color[(uint8_t)level];
        batchWrite((uint8_t*)str_tmp, color_str_len);
        //device->write(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
    }

    /* Info
     * split into two part, or snprintf will crash under windows
     * */
    ret = snprintf(trace_buffer,TRACE_BUFFER_SIZE - 3,
                          "%s  "  /* Level */
                          "%lu "  /* Timestamp */
                          "(+%lu us) "  /* Timestamp Diff */
                        , level_name[(uint8_t)level]
                        , timestamp
                        , (uint64_t)(timestamp - timestamp_last)
                        );
    if(ret >= 0){
        trace_buffer[ret] = 0;
        batchWrite((uint8_t*)trace_buffer, ret + 1);
    }

    timestamp_last = timestamp;

    ret = snprintf(trace_buffer,TRACE_BUFFER_SIZE - 3,"[%s]  " , tag);
    if(ret >= 0){
        trace_buffer[ret] = 0;
        batchWrite((uint8_t*)trace_buffer, ret + 1);
    }


    /* Content */
    ret = vsnprintf(trace_buffer,TRACE_BUFFER_SIZE - 3 , format, arg_ptr);

    if(ret >= 0){
        trace_buffer[ret] = 0;
        batchWrite((uint8_t*)trace_buffer,  ret + 1);
    }

    /* Color */
    if(enable_color)
    {
        str_tmp = ANSI_COLOR_RESET;
        batchWrite((uint8_t*)str_tmp, color_str_len);
    }

    /* 进行跨平台的强制终端输出。*/
#ifdef SYSTYPE_FULL_OS
    std::cout << std::endl;
#else
    batchWrite((uint8_t*)"\n", 1);
#endif

#endif //ENABLE_TRACE
    return ret;
}

int Tracer::printf(Level level, char *format, ...) {

    va_list arg_ptr;
    va_start(arg_ptr, format);
    int ret = this->vprintf(level, format, arg_ptr);
    va_end(arg_ptr);

    return ret;
}

