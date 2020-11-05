//
// Created by sdong on 2020/10/27.
//

#include "TestUtils.hpp"

#include "test_ServoDict.hpp"

using namespace libfcn_v2;

namespace rto_io_test{

    TEST(RealtimeObject, setget){
        int angle_idx = 10;

        SerDesDictVal<uint32_t> Angle(angle_idx);

        auto rto = Angle;

        ASSERT_EQ(rto.data_size, 4);
        ASSERT_EQ(rto.index, angle_idx);

        rto << 200;

        uint32_t a;

        rto >> a;

        ASSERT_EQ(a, 200);
    }

    TEST(RealtimeObject, Dict){
        cout << "sizeof(libfcn_v2_test::TestRODict) = " << sizeof
                (fcnmsg::test_ServoPubSubDict) << endl;

        ASSERT_EQ(sizeof(SerDesDictValHandle), 4);

        auto angle_msg = fcnmsg::test_ServoPubSubDict.angle;
        angle_msg << 100;
        int32_t angle = 0;
        angle_msg >> angle;
        ASSERT_EQ(angle, 100);

        decltype(fcnmsg::test_ServoPubSubDict)::Buffer buffer;
        decltype(fcnmsg::test_ServoPubSubDict) dict;

        dict.serialize(angle_msg, &buffer);

        ASSERT_EQ(buffer.angle, 100);
        ASSERT_EQ(dict.deserialize(angle_msg, &buffer).data, 100);

        cout << "pass!" << endl;
    }
}
