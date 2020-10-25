//
// Created by sdong on 2019/11/14.
//
#include "RealtimeObject.hpp"

#ifndef TESTRODICT_HPP
#define TESTRODICT_HPP

namespace libfcn_v2_test{
    struct _idl_MsgMap{
        uint16_t speed;
        uint16_t angle;
        uint16_t current;
    };

    class TestRODict : public libfcn_v2::ObjectDict{
    public:
        /*可以直接访问的元信息*/
        libfcn_v2::RtoDictItemNoCb<int16_t> speed;
        libfcn_v2::RtoDictItemNoCb<int32_t> angle;
        libfcn_v2::RtoDictItemNoCb<int8_t> current;
        libfcn_v2::RtoDictItemNoCb<int32_t> target_angle;

        TestRODict() : ObjectDict(4),

                       speed(0),
                       angle(1),
                       current(2),
                       target_angle(3){

            obj_dict[this->speed.index] = &this->speed;
            obj_dict[this->angle.index] = &this->angle;
            obj_dict[this->current.index] = &this->current;
            obj_dict[this->target_angle.index] = &this->target_angle;
        }

        ~TestRODict() override = default;
    };

}

#endif //TESTRODICT_HPP
