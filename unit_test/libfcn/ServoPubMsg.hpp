//
// Created by sdong on 2019/11/14.
//
#include "libfcn/SerDesDict.hpp"

#ifndef TESTSERVORTODICT_HPP
#define TESTSERVORTODICT_HPP

namespace fcnmsg{


#pragma pack(2)
    struct ServoPubMsgIn_C : public libfcn_v2::SerDesDict{
    public:
        /*可以直接访问的元信息*/
        libfcn_v2::SerDesDictVal<int32_t> target_angle;
        libfcn_v2::SerDesDictVal<int16_t> mode;

        ServoPubMsgIn_C() :
        SerDesDict(2,&target_angle)
            , target_angle(0)
            , mode        (1)
        {
            val_offset_key_table[target_angle.index] = (uint8_t *)&target_angle
                                                       - (uint8_t*)p_first_val;
            val_offset_key_table[mode.index] = (uint8_t *)&mode
                                               - (uint8_t*)p_first_val;

            Buffer buffer_tmp;

            target_angle.buffer_offset = (uint8_t *)&buffer_tmp.target_angle - (uint8_t *)&buffer_tmp;
            mode.buffer_offset = (uint8_t *)&buffer_tmp.mode - (uint8_t *)&buffer_tmp;
        }


        struct Buffer{
            int32_t target_angle;
            int16_t mode;
        };

        inline void* createBuffer() override{
            auto buffer = new Buffer();

            utils::memset(buffer, 0, sizeof(Buffer));

            return buffer;
        }
    };
#pragma pack(0)

    extern ServoPubMsgIn_C ServoPubMsgIn;


#pragma pack(2)
    struct ServoPubMsgOut_C : public libfcn_v2::SerDesDict{
    public:
        /*可以直接访问的元信息*/
        libfcn_v2::SerDesDictVal<int16_t> speed;
        libfcn_v2::SerDesDictVal<int32_t> angle;
        libfcn_v2::SerDesDictVal<int8_t>  current;

        ServoPubMsgOut_C() :
            SerDesDict(3, &speed)

            , speed   (0)
            , angle   (1)
            , current (2)
        {
            val_offset_key_table[speed.index] = (uint8_t *)&speed
                                                - (uint8_t*)p_first_val;
            val_offset_key_table[angle.index] = (uint8_t *)&angle
                                                - (uint8_t*)p_first_val;
            val_offset_key_table[current.index] = (uint8_t *)&current
                                                  - (uint8_t*)p_first_val;

            Buffer buffer_tmp;

            speed.buffer_offset = (uint8_t *)&buffer_tmp.speed - (uint8_t *)&buffer_tmp;
            angle.buffer_offset = (uint8_t *)&buffer_tmp.angle - (uint8_t *)&buffer_tmp;
            current.buffer_offset = (uint8_t *)&buffer_tmp.current - (uint8_t *)&buffer_tmp;
        }


        struct Buffer{
            int16_t speed;
            int32_t angle;
            int8_t  current;
        };

        inline void* createBuffer() override{
            auto buffer = new Buffer();

            utils::memset(buffer, 0, sizeof(Buffer));

            return buffer;
        }
    };
#pragma pack(0)

    extern ServoPubMsgOut_C ServoPubMsgOut;
}



#endif //TESTSERVORTODICT_HPP
