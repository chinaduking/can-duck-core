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

    class TestRODict : public libfcn_v2::RealtimeObjectDict{
    public:
        /*可以直接访问的元信息*/
        libfcn_v2::RealtimeObject<int16_t> speed;
        libfcn_v2::RealtimeObject<int32_t> angle;
        libfcn_v2::RealtimeObject<int8_t> current;
        libfcn_v2::RealtimeObject<int32_t> target_angle;

        static const uint16_t DICT_SIZE = 4;

        TestRODict() : RealtimeObjectDict(DICT_SIZE){

#if 0
            MSG_METAINFO_INIT_BEGIN;


//            MSG_METAINFO_INIT(_idl_MsgMap, speed);
//            MSG_METAINFO_INIT(_idl_MsgMap, angle);
//            MSG_METAINFO_INIT(_idl_MsgMap, current);
#endif

#if 1
            uint16_t index = 0;

            speed.index = index;
            obj_dict[index] = &this->speed;
            index ++;

            angle.index = index;
            obj_dict[index] = &this->angle;
            index ++;

            current.index = index;
            obj_dict[index] = &this->current;
            index ++;

            target_angle.index = index;
            obj_dict[index] = &this->target_angle;
            index ++;
#endif
        }

        ~TestRODict() override = default;
    };

}

#endif //TESTRODICT_HPP
