//
// Created by sdong on 2020/10/15.
//

#include "TestUtils.hpp"
#include "RealtimeObject.hpp"
#include "test_ServoRTO.hpp"
using namespace libfcn_v2;

namespace rto_test{

    /*
     * 测试实时对象字典从二进制数据写入
     **/
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

    TEST(RtoDict, localWrite){
        decltype(fcnmsg::test_ServoRTO) testRoDict_src;
        decltype(fcnmsg::test_ServoRTO) testRoDict_dest;

        SingleWriteTest(speed,          0x5566);
        SingleWriteTest(angle,          0x55667788);
        SingleWriteTest(current,        0x56);
        SingleWriteTest(target_angle,   0x55667788);

        cout << "pass!" << endl;
    }


    /*
     * 测试实时对象字典在共享内存上进行类型安全的读写
     **/
    const int OWNER_ADDR = 0x07;
    RtoDictManager rtoDictManager(10);

    class Node{
    public:
        Node(){
            rto_dict = rtoDictManager
                    .create<libfcn_v2_test::test_ServoRTO::Buffer>
                    (OWNER_ADDR);
        }
        virtual void spin(){ }

    protected:
        libfcn_v2_test::test_ServoRTO::Buffer* rto_dict;
    };

    class Node07 : public Node{
    public:
        Node07():Node(){ rto_dict->angle = 200; }

        void spin() override {
            rto_dict->angle = rto_dict->angle + 30;
        }
    };

    class Node02 : public Node{
    public:
        Node02():Node(){}
        void spin() override {
            auto angle = rto_dict->angle;

            //ASSERT_EQ(angle.data, 200);
            cout << "angle.data = " << angle << endl;
        }
    };

    TEST(RtoDict, LocalShm){
        Node07 node_07;
        Node02 node_02;

        for(int i = 0; i < 3; i ++){
            node_07.spin();
            node_02.spin();
        }

    }

}


/*
 * 测试实时对象字典使用网络进行传输
 **/
#include "NetworkLayer.hpp"

#include "utils/PosixSerial.hpp"
#include "utils/Tracer.hpp"

#include "test_ServoRTO.hpp"
#include "FrameUtils.hpp"
#include "SimpleSerialNode.hpp"

using namespace libfcn_v2;
using namespace utils;

namespace network_test {

#define SERVO_ADDR 0x02

    TEST(RTO, RtoServoNode) {
        int local_addr = SERVO_ADDR;

        Node fcn_node(0);

        DataLinkFrame frame_tmp;

        auto rto_channel = fcn_node
                .network_layer->rto_network_handler.
                createChannel<decltype(fcnmsg::test_ServoRTO)>(local_addr);

        fcn_node.spin();

        uint32_t cnt = 0;

        for(int __i = 0; __i < 1; ){
            auto speed_msg = fcnmsg::test_ServoRTO.speed;
            speed_msg << cnt;
            rto_channel->publish(speed_msg);

            auto angle_msg = fcnmsg::test_ServoRTO.angle;
            angle_msg << cnt;
            rto_channel->publish(angle_msg);

            auto current_msg = fcnmsg::test_ServoRTO.current;
            current_msg << cnt;
            rto_channel->publish(current_msg);


//            coutinuousWriteFrameBuilder(&frame_tmp, rto_dict,
//                                        rto_dict->speed.index,
//                                        rto_dict->current.index,
//                                        local_addr,
//                                        0x00,
//                                        static_cast<uint8_t>(OpCode::RTO_PUB));

            frame_tmp.src_id = local_addr;
            frame_tmp.dest_id = 0x00; /*ANY*/

            cout << DataLinkFrameToString(frame_tmp) << endl;

            fcn_node.frame_dev->write(&frame_tmp);

            //fcn_node.spin();
            perciseSleep(0.1);

            cnt ++;
        }
    }


    TEST(RTO, RtoHostNode) {
        Node fcn_node(1);
        Tracer tracer(true);
        tracer.setFilter(Tracer::INFO);

        int servo_addr = SERVO_ADDR;

        auto servo_rto_channel = fcn_node.network_layer->rto_network_handler.
                createChannel<libfcn_v2_test::test_ServoRTO>(servo_addr);

        fcn_node.spin();

        auto angle = servo_rto_channel->fetchBuffer(test_ServoRTO::angle).data;

        for(int __i = 0; __i < 1; ){
//            fcn_node.spin();
            perciseSleep(0.1);

            tracer.print(Tracer::WARNING, "servo: speed = %d, angle = %d"
                                          ", current = %d \n",
                 servo_rto_channel->fetchBuffer(test_ServoRTO::speed).data,
                 servo_rto_channel->fetchBuffer(test_ServoRTO::angle).data,
                 servo_rto_channel->fetchBuffer(test_ServoRTO::angle).data);
        }

        fcn_node.join();
    }

}