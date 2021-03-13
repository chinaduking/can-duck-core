//
// Created by sdong on 2019/11/14.
//
#include "SerDesDict.hpp"
#include "Array.hpp"
#ifndef TESTSERVORTODICT_HPP
#define TESTSERVORTODICT_HPP

namespace duckmsg{


#pragma pack(2)
    struct ServoService_C : public can_duck::SerDesDict{
    public:
        /*可以直接访问的元信息*/
        can_duck::DictItem<emlib::Array<uint8_t, 8>> serial_num;
        can_duck::DictItem<int16_t> mode;
        can_duck::DictItem<float> kp;
        can_duck::DictItem<float> kd;


        ServoService_C() :
                SerDesDict(4, &serial_num)
                , serial_num(0)
                , mode      (1)
                , kp        (2)
                , kd        (3)
        {
            val_offset_key_table[serial_num.index] = (uint8_t *)&serial_num
                                                       - (uint8_t*)p_first_val;
            val_offset_key_table[mode.index] = (uint8_t *)&mode
                                               - (uint8_t*)p_first_val;
            val_offset_key_table[kp.index] = (uint8_t *)&kp
                                               - (uint8_t*)p_first_val;
            val_offset_key_table[kd.index] = (uint8_t *)&kd
                                             - (uint8_t*)p_first_val;


            Buffer buffer_tmp;

            serial_num.buffer_offset = (uint8_t *)&buffer_tmp.serial_num - (uint8_t *)&buffer_tmp;
            mode.buffer_offset = (uint8_t *)&buffer_tmp.mode - (uint8_t *)&buffer_tmp;
            kp.buffer_offset = (uint8_t *)&buffer_tmp.kp - (uint8_t *)&buffer_tmp;
            kd.buffer_offset = (uint8_t *)&buffer_tmp.kd - (uint8_t *)&buffer_tmp;
        }


        struct Buffer{
            emlib::Array<uint8_t, 8> serial_num;
            int16_t mode;
            float kp;
            float kd;
        };

        inline void* createBuffer() override{
            auto buffer = new Buffer();

            emlib::memset(buffer, 0, sizeof(Buffer));

            return buffer;
        }
    };
#pragma pack(0)

    extern ServoService_C servo_service;



#pragma pack(2)
    struct ServoMsgI_C : public can_duck::SerDesDict{
    public:
        /*可以直接访问的元信息*/
        can_duck::DictItem<int32_t> target_angle;
        can_duck::DictItem<int16_t> mode;

        ServoMsgI_C() :
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

            emlib::memset(buffer, 0, sizeof(Buffer));

            return buffer;
        }
    };
#pragma pack(0)

    extern ServoMsgI_C servo_msg_i;


#pragma pack(2)
    struct ServoMsgO_C : public can_duck::SerDesDict{
    public:
        /*可以直接访问的元信息*/
        can_duck::DictItem<int16_t> speed;
        can_duck::DictItem<int32_t> angle;
        can_duck::DictItem<int8_t>  current;

        ServoMsgO_C() :
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

            emlib::memset(buffer, 0, sizeof(Buffer));

            return buffer;
        }
    };
#pragma pack(0)

    extern ServoMsgO_C servo_msg_o;
}



#endif //TESTSERVORTODICT_HPP
