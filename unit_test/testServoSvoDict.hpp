//
// Created by sdong on 2019/11/14.
//
#include "DataObjects.hpp"

#ifndef TESTSERVOSVODICT_HPP
#define TESTSERVOSVODICT_HPP

namespace libfcn_v2_test{
//    struct _idl_MsgMap{
//        uint16_t speed;
//        uint16_t angle;
//        uint16_t current;
//    };

#pragma pack(2)
    struct testServoSvoDict : public libfcn_v2::ServiceObjectDict{
    public:
        /*可以直接访问的元信息*/
        libfcn_v2::ServiceObject<int16_t> speed;
        libfcn_v2::ServiceObject<int32_t> angle;
        libfcn_v2::ServiceObject<int8_t>  current;
        libfcn_v2::ServiceObject<int32_t> target_angle;
        libfcn_v2::ServiceObject<int16_t> mode;

        testServoSvoDict() : ServiceObjectDict(5),

                             speed   (0, 0),
                             angle   (1, 0),
                             current (2, 0),
                             target_angle(3, 1),
                             mode    (4, 1)

                       {

            obj_dict[this->speed.index] = &this->speed;
            obj_dict[this->angle.index] = &this->angle;
            obj_dict[this->current.index] = &this->current;
            obj_dict[this->target_angle.index] = &this->target_angle;
            obj_dict[this->mode.index] = &this->mode;

            /*self test*/
            USER_ASSERT(obj_dict[5]->index == obj_dict.size
            () - 1);
        }
    };
#pragma pack(0)

}

#endif //TESTSERVOSVODICT_HPP
