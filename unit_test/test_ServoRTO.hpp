//
// Created by sdong on 2019/11/14.
//
#include "DataObjects.hpp"

#ifndef TESTSERVORTODICT_HPP
#define TESTSERVORTODICT_HPP

namespace fcnmsg{


#pragma pack(2)
    struct test_ServoRTO_C : public libfcn_v2::ObjectDictMM{
    public:
        /*可以直接访问的元信息*/
        libfcn_v2::ObjectPrototype<int16_t> speed;
        libfcn_v2::ObjectPrototype<int32_t> angle;
        libfcn_v2::ObjectPrototype<int8_t>  current;
        libfcn_v2::ObjectPrototype<int32_t> target_angle;
        libfcn_v2::ObjectPrototype<int16_t> mode;

        test_ServoRTO_C(void* p_buffer = nullptr) : ObjectDictMM(
            5,
            &speed,
            p_buffer)
        ,
        speed   (0),
        angle   (1),
        current (2),
        target_angle(3),
        mode    (4)
        {
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


        struct Buffer{
            int16_t speed;
            int32_t angle;
            int8_t  current;
            int32_t target_angle;
            int16_t mode;
        };

        void* createBuffer() override;
    };
#pragma pack(0)

    extern test_ServoRTO_C test_ServoRTO;
}



#endif //TESTSERVORTODICT_HPP
