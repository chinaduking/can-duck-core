//
// Created by sdong on 2019/11/14.
//
#include "SerDesDict.hpp"

#ifndef TESTSERVOSVODICT_HPP
#define TESTSERVOSVODICT_HPP

namespace libfcn_v2_test{
    struct _idl_MsgMap{
        int16_t speed;
        int32_t angle;
        int8_t  current;
        int32_t target_angle;
        int16_t mode;
    };

#pragma pack(2)
    struct testServoSvoDict : public libfcn_v2::ServiceObjectDict{
    public:
        /*可以直接访问的元信息*/
        static libfcn_v2::ServiceObject<int16_t> speed;
        static libfcn_v2::ServiceObject<int32_t> angle;
        static libfcn_v2::ServiceObject<int8_t>  current;
        static libfcn_v2::ServiceObject<int32_t> target_angle;
        static libfcn_v2::ServiceObject<int16_t> mode;

        testServoSvoDict() : ServiceObjectDict(5)

                       {
//
//            obj_dict[this->speed.index]     = &this->speed;
//            obj_dict[this->angle.index]     = &this->angle;
//            obj_dict[this->current.index]   = &this->current;
//            obj_dict[this->target_angle.index] = &this->target_angle;
//            obj_dict[this->mode.index]      = &this->mode;
//
//            /*self test*/
//            USER_ASSERT(obj_dict[5]->index
//                == obj_dict.size() - 1);
        }

        _idl_MsgMap buffer;
    };


    libfcn_v2::ServiceObject<int16_t> testServoSvoDict::speed   (0, 0);
    libfcn_v2::ServiceObject<int32_t> testServoSvoDict::angle   (1, 0);
    libfcn_v2::ServiceObject<int8_t>  testServoSvoDict::current (2, 0);
    libfcn_v2::ServiceObject<int32_t> testServoSvoDict::target_angle(3, 1);
    libfcn_v2::ServiceObject<int16_t> testServoSvoDict::mode    (4, 1);

#pragma pack(0)

}

#endif //TESTSERVOSVODICT_HPP
