//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_TESTUTILS_HPP
#define LIBFCN_V2_TESTUTILS_HPP

#include "gtest/gtest.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>
#include <mutex>


using namespace std;


#include "libfcn/DataLinkLayer.hpp"

namespace libfcn_v2{
    inline bool DataLinkFrameCompare(FcnFrame& frame1, FcnFrame&
    frame2){
        if(frame1.payload_len != frame2.payload_len){
            return false;
        }

        if(frame1.msg_id != frame2.msg_id){
            return false;
        }

        if(frame1.src_id != frame2.src_id){
            return false;
        }

        if(frame1.dest_id != frame2.dest_id){
            return false;
        }

        if(frame1.op_code != frame2.op_code){
            return false;
        }

        return memcmp(frame1.payload, frame2.payload, frame1.payload_len) == 0;
    }

    inline bool DataLinkFramePayloadCompare(FcnFrame& frame1,
                                            FcnFrame& frame2){
        if(frame1.payload_len != frame2.payload_len){
            return false;
        }

        return memcmp(frame1.payload, frame2.payload, frame1.payload_len) == 0;
    }

}

#endif //LIBFCN_V2_TESTUTILS_HPP
