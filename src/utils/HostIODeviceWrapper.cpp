//
// Created by sdong on 2019/11/17.
//

#include "HostIODeviceWrapper.hpp"
#include <cstdio>


using namespace std;
using namespace utils;


/*stdout wrapper*/
int32_t StdoutIODviceWrapper::write(const uint8_t *data, uint32_t len){
    printf("%s", data);
    return 0;
}

/*No Implement Methods*/
int32_t StdoutIODviceWrapper::read(uint8_t *data, uint32_t len){
    return 0;
}

FileIODviceWrapper::FileIODviceWrapper(std::string path) {}

/*file io wrapper*/
int32_t FileIODviceWrapper::read(uint8_t *data, uint32_t len){
    //TODO: impl this!
    return 0;
}
int32_t FileIODviceWrapper::write(const uint8_t *data, uint32_t len){
    //TODO: impl this!
    return 0;
}