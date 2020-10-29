//
// Created by sdong on 2020/10/29.
//

#include "test_ServoRTO.hpp"


using namespace libfcn_v2;

namespace fcnmsg{
    test_ServoRTO_C  test_ServoRTO;

    void * test_ServoRTO_C::createBuffer() {
        return new test_ServoRTO_C::Buffer;
    }


    void test_ServoRTO_C::init() {
        obj_base_offset[speed.index] = (uint8_t *)&speed
                                       - (uint8_t*)p_first_obj;
        obj_base_offset[angle.index] = (uint8_t *)&angle
                                       - (uint8_t*)p_first_obj;
        obj_base_offset[current.index] = (uint8_t *)&current
                                         - (uint8_t*)p_first_obj;
        obj_base_offset[target_angle.index] = (uint8_t *)&target_angle
                                              - (uint8_t*)p_first_obj;
        obj_base_offset[mode.index] = (uint8_t *)&mode
                                      - (uint8_t*)p_first_obj;

        Buffer buffer_tmp;

        speed.buffer_offset = (uint8_t *)&buffer_tmp.speed - (uint8_t *)&buffer_tmp;
        angle.buffer_offset = (uint8_t *)&buffer_tmp.angle - (uint8_t *)&buffer_tmp;
        current.buffer_offset = (uint8_t *)&buffer_tmp.current - (uint8_t *)&buffer_tmp;
        target_angle.buffer_offset = (uint8_t *)&buffer_tmp.target_angle - (uint8_t *)&buffer_tmp;
        mode.buffer_offset = (uint8_t *)&buffer_tmp.mode - (uint8_t *)&buffer_tmp;
    }
}
