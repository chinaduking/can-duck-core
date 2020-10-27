//
// Created by sdong on 2020/10/15.
//

#include "TestUtils.hpp"
#include "RealtimeObject.hpp"
#include "testServoRtoDict.hpp"
using namespace libfcn_v2;

namespace rto_test{

    #define SingleWriteTest(item, data_) do{                                \
               testRoDict_src.item << data_;                                \
               uint16_t index =  testRoDict_src.item.index;                 \
               uint16_t len = testRoDict_src.item.data_size;                \
               uint8_t* data = (uint8_t*)testRoDict_src.item.getDataPtr();  \
               RtoDictContinuousWrite(&testRoDict_dest, index, data, len);  \
               ASSERT_EQ(testRoDict_dest.item.data, data_);                 \
               std::cout << "testRoDict_dest." << #item << " = "            \
                << testRoDict_dest.item.data << std::endl;                  \
        } while(0)

    TEST(RealtimeObject, singleWrite){
        libfcn_v2_test::testServoRtoDict testRoDict_src;
        libfcn_v2_test::testServoRtoDict testRoDict_dest;

        SingleWriteTest(speed,          0x5566);
        SingleWriteTest(angle,          0x55667788);
        SingleWriteTest(current,        0x56);
        SingleWriteTest(target_angle,   0x55667788);

        cout << "pass!" << endl;
    }


    const int OWNER_ADDR = 0x07;

    class Node{
    public:
        Node(){
            rto_dict = RtoDictManager::getInstance()
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



#include "NetworkLayer.hpp"

#include "utils/PosixSerial.hpp"
#include "utils/Tracer.hpp"

#include "testServoRtoDict.hpp"
#include "FrameUtils.hpp"
#include "SimpleSerialNode.hpp"

using namespace libfcn_v2;
using namespace utils;

namespace network_test {

#define SERVO_ADDR 0x02

    TEST(NetworkLayer, RtoServoNode) {
        int local_addr = SERVO_ADDR;

        Node fcn_node(0);

        DataLinkFrame frame_tmp;

        auto rto_dict = fcn_node.network_layer->rto_network_handler.
                bindDictToChannel<libfcn_v2_test::testServoRtoDict>(local_addr);

        fcn_node.spin();

        uint32_t cnt = 0;

        while(1){
            rto_dict->speed << cnt;
            rto_dict->angle << cnt;
            rto_dict->current << cnt;

            coutinuousWriteFrameBuilder(&frame_tmp, rto_dict,
                                        rto_dict->speed.index,
                                        rto_dict->current.index,
                                        local_addr,
                                        0x00,
                                        static_cast<uint8_t>(OpCode::RTO_PUB));

            frame_tmp.src_id = local_addr;
            frame_tmp.dest_id = 0x00; /*ANY*/

            cout << DataLinkFrameToString(frame_tmp) << endl;

            fcn_node.frame_dev->write(&frame_tmp);

            //fcn_node.spin();
            perciseSleep(0.1);

            cnt ++;
        }
    }


    TEST(NetworkLayer, RtoHostNode) {
        Node fcn_node(1);
        Tracer tracer(true);
        tracer.setFilter(Tracer::INFO);

        int servo_addr = SERVO_ADDR;

        auto servo_rto_dict = fcn_node.network_layer->rto_network_handler.
                bindDictToChannel<libfcn_v2_test::testServoRtoDict>(servo_addr);

        fcn_node.spin();

        while(1){
//            fcn_node.spin();
            perciseSleep(0.1);

            tracer.print(Tracer::WARNING, "servo: speed = %d, angle = %d"
                                          ", current = %d \n",
                         servo_rto_dict->speed.data,
                         servo_rto_dict->angle.data,
                         servo_rto_dict->current.data);
        }

        fcn_node.join();
    }

}