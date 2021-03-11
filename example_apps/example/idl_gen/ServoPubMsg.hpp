//
// Created by sdong on 2019/11/14.
//
#include "libfcn/SerDesDict.hpp"

#ifndef TESTSERVORTODICT_HPP
#define TESTSERVORTODICT_HPP

namespace duckmsg{


#pragma pack(2)
    struct ServoPubMsgIn_C : public can_duck::SerDesDict{
    public:
        /*可以直接访问的元信息*/
        can_duck::SerDesDictVal<int16_t> speed;
        can_duck::SerDesDictVal<int32_t> angle;
        can_duck::SerDesDictVal<int8_t>  current;
        can_duck::SerDesDictVal<int32_t> target_angle;
        can_duck::SerDesDictVal<int16_t> mode;

        ServoPubMsgIn_C() : SerDesDict(
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
            return new ServoPubMsgIn_C::Buffer;
        }
    };
#pragma pack(0)

    extern ServoPubMsgIn_C ServoPubMsgIn;
}



#endif //TESTSERVORTODICT_HPP
