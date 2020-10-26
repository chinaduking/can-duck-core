//
// Created by sdong on 2019/11/11.
//
#include "utils/DataVerify.hpp"
#include "gtest/gtest.h"

using namespace utils;
using namespace std;


namespace dataver_test{
    /*  buffer wait test  */
    uint8_t test_buffer[8] = {
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
    };
    uint16_t test_buffer_len = 8;

    /*
     * expect return value of every verify method
     * */
    uint16_t Crc16_retv = 0x7F69;
    uint8_t CheckSum1ByteIn1ByteOut_retv = 0xDB;//219;
    uint16_t CheckSum1ByteIn2ByteOut_retv = 0xFCDB;//64731;
    uint32_t CheckSum1ByteIn4ByteOut_retv = 0xFFFFFCDB;//4294966491;
    uint32_t CheckSum4ByteIn4ByteOut_retv = 0x33353739;//859125561;

    TEST(DataVerifyTest, Crc16){
        ASSERT_EQ(Crc16_retv, Crc16(test_buffer, test_buffer_len));
    }
    TEST(DataVerifyTest, CheckSum1ByteIn1ByteOut_retv){
        ASSERT_EQ(CheckSum1ByteIn1ByteOut_retv,
                  CheckSum1x1(test_buffer, test_buffer_len));
    }
    TEST(DataVerifyTest, CheckSum1ByteIn2ByteOut_retv){
        ASSERT_EQ(CheckSum1ByteIn2ByteOut_retv,
                  CheckSum1x2(test_buffer, test_buffer_len));
    }
    TEST(DataVerifyTest, CheckSum1ByteIn4ByteOut_retv){
        ASSERT_EQ(CheckSum1ByteIn4ByteOut_retv,
                  CheckSum1x4(test_buffer, test_buffer_len));
    }
    TEST(DataVerifyTest, CheckSum4ByteIn4ByteOut_retv){
        ASSERT_EQ(CheckSum4ByteIn4ByteOut_retv,
                  CheckSum4x4(test_buffer, test_buffer_len));
    }


}