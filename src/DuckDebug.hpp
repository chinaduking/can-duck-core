//
// Created by 董世谦 on 2021/3/12.
//

#ifndef CAN_DUCK_CORE_DUCKDEBUG_HPP
#define CAN_DUCK_CORE_DUCKDEBUG_HPP
#include "LLComDevice.hpp"
#include "Common.hpp"

#ifdef SYSTYPE_FULL_OS

#include <string>

#endif

namespace can_duck{
#ifdef SYSTYPE_FULL_OS
    std::string frame2stdstr(ServiceFrame& frame);
#endif
    uint32_t frame2strbuf(ServiceFrame& frame, char* buffer, uint32_t buffer_size);

}
#endif //CAN_DUCK_CORE_DUCKDEBUG_HPP
