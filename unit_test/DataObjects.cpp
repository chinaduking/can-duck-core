//
// Created by sdong on 2020/10/27.
//

#include "TestUtils.hpp"

#include "testServoRtoDict.hpp"

using namespace libfcn_v2;

namespace rto_io_test{
    TEST(RealtimeObject, setget){
        int angle_idx = 10;

        RealtimeObjectNoCb<uint32_t> Angle(angle_idx);

        auto rto = Angle;

        ASSERT_EQ(rto.data_size, 4);
        ASSERT_EQ(rto.index, angle_idx);

        rto << 200;

        uint32_t a;

        rto >> a;

        ASSERT_EQ(a, 200);
    }

    TEST(RealtimeObject, Dict){
        libfcn_v2_test::testServoRtoDict testRoDict;
        testRoDict.angle << 100;

        cout << "sizeof(libfcn_v2_test::TestRODict) = " << sizeof
                (libfcn_v2_test::testServoRtoDict) << endl;

        ASSERT_EQ(sizeof(RealtimeObjectBase), 4);

        cout<< "sizeof(RealtimeObjectBase) = "
            << sizeof(RealtimeObjectBase) << endl;

        cout << "sizeof(libfcn_v2_test::TestRODict::angle) = " << sizeof
        (libfcn_v2_test::testServoRtoDict::angle) << endl;

        cout<< "sizeof(FcnCallbackInterface*) = " << sizeof
                (FcnCallbackInterface*) << endl;


        int32_t angle = 0;
        testRoDict.angle >> angle;
        ASSERT_EQ(angle, 100);


        ASSERT_EQ(testRoDict.speed.index, 0);
        ASSERT_EQ(testRoDict.angle.index, 1);
        ASSERT_EQ(testRoDict.current.index, 2);


        ASSERT_EQ(testRoDict.speed.data_size, 2);
        ASSERT_EQ(testRoDict.angle.data_size, 4);
        ASSERT_EQ(testRoDict.current.data_size, 1);

        cout << "pass!" << endl;
    }

    struct testCallbackObj : public FcnCallbackInterface {
        void callback(void *data, uint8_t ev_code) override {
            cout << "testCallbackObj! data = " << *(uint32_t*)data << endl;
        }
    };

    TEST(RealtimeObject, callback) {
        int angle_idx = 10;
        RealtimeObjectCb<uint32_t> angle(angle_idx);

        angle << 230;

        auto cb_handle = new testCallbackObj();
        angle.callback = cb_handle;

        printf("sizeof(RtoDictItemBase) = %d\n", sizeof(RealtimeObjectBase));

        printf("sizeof(RtoDictItemCb<uint32_t>) = %d\n", sizeof(RealtimeObjectCb<uint32_t>));

        printf("sizeof(RtoDictItemCb<uint8_t>) = %d\n", sizeof(RealtimeObjectCb<uint8_t>));

        printf("sizeof(RtoDictItemNoCb<uint8_t>) = %d\n", sizeof
                (RealtimeObjectNoCb<uint8_t>));

        printf("sizeof(RtoDictItemNoCb<uint32_t>) = %d\n", sizeof(RealtimeObjectNoCb<uint32_t>));

        printf("sizeof(testCallbackObj*) = %d\n", sizeof(testCallbackObj*));


        printf("addr of class testCallbackObj instance = %p\n", angle.callback);
        printf("addr of Angle instance = %p\n", &angle);
        printf("addr of Angle.callback instance = %p\n", &angle.callback);

        printf("value of Angle.callback instance = %lx\n", angle.callback);
        printf("addr of (FcnCallbackInterface*)(*(&angle.callback) = %p\n",
               (FcnCallbackInterface*)(*(&angle.callback)));


        printf("value of Angle.callback instance = %p\n", angle.getCallbackPtr());

        ASSERT_EQ(angle.getCallbackPtr(), cb_handle);

        uint32_t test_data = 200;

        printf("call from ObjDictItemCb:\n");
        angle.callback->callback(&test_data, 1);

        printf("call from ObjDictItemBase:\n");
        FcnCallbackInterface* cb = angle.getCallbackPtr();

        cb->callback(&test_data, 1);

        printf("data value: %d\n", *(uint32_t*)(angle.getDataPtr()));


    }
}


#include "testServoSvoDict.hpp"


using namespace libfcn_v2;

namespace svo_io_test{
    TEST(Svo, io){
        libfcn_v2_test::testServoSvoDict dict;

        ASSERT_EQ(dict.angle.wr_access, 0);
        ASSERT_EQ(dict.target_angle.wr_access, 1);
    }
}