//
// Created by sdong on 2019/11/14.
//

#include "Tracer.hpp"
#include <cstdio>
#include <cstdarg>
#include <iostream>

using namespace utils;

#define TRACE_BUFFER_SIZE 1024
#define MAX_BINDING_OUTPUT_DEVICE 3


static char* level_name[] = {
        (char*) "N",
        (char*) "I",
        (char*) "V",
        (char*) "!W",
        (char*) "!!E",
        (char*) "!!!F"
};

#define ANSI_COLOR_RED     (char*) "\x1b[31m"
#define ANSI_COLOR_GREEN   (char*) "\x1b[32m"
#define ANSI_COLOR_YELLOW  (char*) "\x1b[33m"
#define ANSI_COLOR_BLUE    (char*) "\x1b[34m"
#define ANSI_COLOR_MAGENTA (char*) "\x1b[35m"
#define ANSI_COLOR_CYAN    (char*) "\x1b[36m"
#define ANSI_COLOR_RESET   (char*) "\x1b[0m"

static char* level_color[] = {
        (char*) "",
        ANSI_COLOR_GREEN,
        ANSI_COLOR_BLUE,
        ANSI_COLOR_YELLOW,
        ANSI_COLOR_RED,
        ANSI_COLOR_MAGENTA
};

Tracer::Tracer(bool enable_color)
    : enable_color(enable_color),  device(MAX_BINDING_OUTPUT_DEVICE){
    tag[0] = 0;


#ifdef SYSTYPE_FULL_OS
    addByteIODeviece(&stdio_wrapper);
#endif //SYSTYPE_FULL_OS
}

void Tracer::setFilter(Level level){

#ifdef SYSTYPE_FULL_OS
    std::lock_guard<std::mutex> lk(update_mutex);
#endif //SYSTYPE_FULL_OS

    if(level > FATAL){
        return;
    }

    filter_level = level;
}

void Tracer::addByteIODeviece(LLByteDevice* device){
#ifdef SYSTYPE_FULL_OS
    std::lock_guard<std::mutex> lk(update_mutex);
#endif //SYSTYPE_FULL_OS

    if(device == nullptr){
        return;
    }

    this->device.push_back(device);
}


void Tracer::setTag(char* tag){

#ifdef SYSTYPE_FULL_OS
    std::lock_guard<std::mutex> lk(update_mutex);
#endif //SYSTYPE_FULL_OS

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

void Tracer::print(Level level, char *format, ...) {

#ifdef SYSTYPE_FULL_OS
    std::lock_guard<std::mutex> lk(update_mutex);
#endif //SYSTYPE_FULL_OS


    if(filter_level == NONE || device.size() == 0){
        return;
    }

    if(level < filter_level || level > FATAL){
        return;
    }

    char* str_tmp;

    /* Color */
    if(enable_color){
        str_tmp = level_color[level];
        batchWrite(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
        //device->write(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
    }

    /* Header */
    str_tmp = (char*) "-->[";
    batchWrite(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
    //device->write(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);

    /* Level */
    str_tmp = level_name[level];
    batchWrite(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
    //device->write(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);

    str_tmp = (char*) "] [";
    batchWrite(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
    //device->write(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);

    /* Tag */
    str_tmp = tag;
    batchWrite(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
    //device->write(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);

    str_tmp = (char*) "]: ";
    batchWrite(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
    //device->write(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);

    /* Content */
    va_list arg_ptr;
    int ret;

    va_start(arg_ptr, format);
    ret = vsprintf(trace_buffer, format, arg_ptr);
    va_end(arg_ptr);

    if(ret >= 0){
        trace_buffer[ret] = 0;
        batchWrite(reinterpret_cast<const uint8_t *>(trace_buffer), strlen(trace_buffer) + 1);
        //device->write(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
    }


    /* Color */
    if(enable_color)
    {
        str_tmp = ANSI_COLOR_RESET;
        batchWrite(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
        //device->write(reinterpret_cast<const uint8_t *>(str_tmp), strlen(str_tmp) + 1);
    }

    //batchWrite(reinterpret_cast<const uint8_t *>("\n"), 2);
    //device->write(reinterpret_cast<const uint8_t *>("\n"), 2);

    std::cout << std::endl;
}


