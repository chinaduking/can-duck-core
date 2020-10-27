//
// Created by sdong on 2019/11/14.
//
#include "DataObjects.hpp"

#ifndef TESTSERVORTODICT_HPP
#define TESTSERVORTODICT_HPP

namespace libfcn_v2_test{
//    struct _idl_MsgMap{
//        uint16_t speed;
//        uint16_t angle;
//        uint16_t current;
//    };

#pragma pack(2)
    struct testServoRtoDict : public libfcn_v2::RealtimeObjectDict{
    public:
        /*可以直接访问的元信息*/
        libfcn_v2::RealtimeObjectNoCb<int16_t> speed;
        libfcn_v2::RealtimeObjectNoCb<int32_t> angle;
        libfcn_v2::RealtimeObjectNoCb<int8_t>  current;
        libfcn_v2::RealtimeObjectNoCb<int32_t> target_angle;
        libfcn_v2::RealtimeObjectNoCb<int16_t> mode;

        testServoRtoDict() : RealtimeObjectDict(5),

                             speed(0),
                             angle(1),
                             current(2),
                             target_angle(3),
                             mode(4)

                       {

            obj_dict[this->speed.index] = &this->speed;
            obj_dict[this->angle.index] = &this->angle;
            obj_dict[this->current.index] = &this->current;
            obj_dict[this->target_angle.index] = &this->target_angle;
            obj_dict[this->mode.index] = &this->mode;
        }

//        ~TestRODict() override = default;
    };
#pragma pack(0)

}

#endif //TESTSERVORTODICT_HPP
