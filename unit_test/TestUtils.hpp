//
// Created by sdong on 2020/10/15.
//

#ifndef can_duck_TESTUTILS_HPP
#define can_duck_TESTUTILS_HPP

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


//#include "DataLinkLayer.hpp"

namespace can_duck{
//    inline bool DataLinkFrameCompare(ServiceFrame& frame1, ServiceFrame&
//    frame2){
//        if(frame1.getPayloadLen() != frame2.getPayloadLen()){
//            return false;
//        }
//
//        if(frame1.srv_id != frame2.srv_id){
//            return false;
//        }
//
//        if(frame1.src_id != frame2.src_id){
//            return false;
//        }
//
//        if(frame1.dest_id != frame2.dest_id){
//            return false;
//        }
//
//        if(frame1.op_code != frame2.op_code){
//            return false;
//        }
//
//        return memcmp(frame1.payload, frame2.payload, frame1.getPayloadLen()) == 0;
//    }
//
//    inline bool DataLinkFramePayloadCompare(ServiceFrame& frame1,
//                                            ServiceFrame& frame2){
//        if(frame1.getPayloadLen() != frame2.getPayloadLen()){
//            return false;
//        }
//
//        return memcmp(frame1.payload, frame2.payload, frame1.getPayloadLen()) == 0;
//    }

}

#endif //can_duck_TESTUTILS_HPP
