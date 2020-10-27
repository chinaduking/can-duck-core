//
// Created by sdong on 2020/10/15.
//

#include "TestUtils.hpp"
#include "RealtimeObject.hpp"
#include "testServoRtoDict.hpp"
using namespace libfcn_v2;

namespace rto_test{

    #define SingleWriteTest(item, data_) do{ \
               testRoDict_src.item << data_;                                 \
               uint16_t index =  testRoDict_src.item.index;                 \
               uint16_t len = testRoDict_src.item.data_size;                \
               uint8_t* data = (uint8_t*)testRoDict_src.item.getDataPtr(); \
               RtoDictContinuousWrite(&testRoDict_dest, index, data, len);               \
               ASSERT_EQ(testRoDict_dest.item.data, data_);                  \
               std::cout << "testRoDict_dest." << #item << " = "             \
                << testRoDict_dest.item.data << std::endl;\
        } while(0)

    TEST(RealtimeObject, singleWrite){
        libfcn_v2_test::testServoRtoDict testRoDict_src;
        libfcn_v2_test::testServoRtoDict testRoDict_dest;

        SingleWriteTest(speed, 0x5566);
        SingleWriteTest(angle, 0x55667788);
        SingleWriteTest(current, 0x56);
        SingleWriteTest(target_angle, 0x55667788);

        cout << "pass!" << endl;
    }



    const int OWNER_ADDR = 0x07;

    class Node{
    public:
        Node(){
            rto_dict = RtoShmManager::getInstance()
                    ->getSharedDict<libfcn_v2_test::testServoRtoDict>(OWNER_ADDR);
        }

        virtual void spin(){ }

    protected:
        libfcn_v2_test::testServoRtoDict* rto_dict;
    };


    class Node07 : public Node{
    public:
        Node07():Node(){
            rto_dict->angle << 200;
        }
        void spin() override {
            rto_dict->angle << rto_dict->angle.data + 30;
        }
    };

    class Node02 : public Node{
    public:
        Node02():Node(){}
        void spin() override {
            auto angle = rto_dict->angle;

            //ASSERT_EQ(angle.data, 200);
            cout << "angle.data = " << angle.data << endl;
        }
    };

    TEST(RealtimeObject, LocalShm){
        Node07 node_07;
        Node02 node_02;

        for(int i = 0; i < 3; i ++){
            node_07.spin();
            node_02.spin();
        }

    }

}
