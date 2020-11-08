//
// Created by sdong on 2019/11/14.
//
#include "SerDesDict.hpp"

#ifndef TESTSERVORTODICT_HPP
#define TESTSERVORTODICT_HPP

namespace fcnmsg{


#pragma pack(2)
    struct test_ServoPubSubDict_C : public libfcn_v2::SerDesDict{
    public:
        /*可以直接访问的元信息*/
        libfcn_v2::SerDesDictVal<int16_t> speed;
        libfcn_v2::SerDesDictVal<int32_t> angle;
        libfcn_v2::SerDesDictVal<int8_t>  current;
        libfcn_v2::SerDesDictVal<int32_t> target_angle;
        libfcn_v2::SerDesDictVal<int16_t> mode;

        test_ServoPubSubDict_C() : SerDesDict(
            5,
            &speed)
        ,
                                   speed   (0),
                                   angle   (1),
                                   current (2),
                                   target_angle(3),
                                   mode    (4)
        {
            val_offset_key_table[speed.index] = (uint8_t *)&speed
                                                - (uint8_t*)p_first_val;
            val_offset_key_table[angle.index] = (uint8_t *)&angle
                                                - (uint8_t*)p_first_val;
            val_offset_key_table[current.index] = (uint8_t *)&current
                                                  - (uint8_t*)p_first_val;
            val_offset_key_table[target_angle.index] = (uint8_t *)&target_angle
                                                       - (uint8_t*)p_first_val;
            val_offset_key_table[mode.index] = (uint8_t *)&mode
                                               - (uint8_t*)p_first_val;

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

        inline void* createBuffer() override{
            return new test_ServoPubSubDict_C::Buffer;
        }
    };
#pragma pack(0)

    extern test_ServoPubSubDict_C test_ServoPubSubDict;
}



#endif //TESTSERVORTODICT_HPP
