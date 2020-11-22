//
// Created by sdong on 2020/10/15.
//

#include "TestUtils.hpp"
#include "libfcn/PubSub.hpp"
#include "ServoPubMsg.hpp"
using namespace libfcn_v2;
#if 0
namespace rto_test{

    /*
     * 测试实时对象字典从二进制数据写入
     **/
    #define SingleWriteTest(item, data_) do{                                \
               buf_src.item = data_;                                \
               uint16_t index =  testRoDict_src.item.index;                 \
               uint16_t len = testRoDict_src.item.data_size;                \
               uint8_t* data = (uint8_t*)testRoDict_src.getBufferDataPtr    \
               (testRoDict_src.item.index);                                 \
                                                                            \
               RtoDictSingleWrite(&testRoDict_dest, index, data, len);      \
                                                                            \
               printf("testRoDict_dest.%s = %X\n", #item,                   \
               testRoDict_dest.read(fcnmsg::test_ServoPubSubDict.item).data);      \
               ASSERT_EQ(testRoDict_dest.read(fcnmsg::test_ServoPubSubDict.item).data,\
               data_) ; \
        } while(0)

    TEST(RtoDict, localWrite){
        decltype(fcnmsg::test_ServoPubSubDict)::Buffer buf_src;
        decltype(fcnmsg::test_ServoPubSubDict) testRoDict_src(&buf_src);

        decltype(fcnmsg::test_ServoPubSubDict)::Buffer buf_dest;
        decltype(fcnmsg::test_ServoPubSubDict) testRoDict_dest(&buf_dest);

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
                    .create<fcnmsg::test_ServoPubSubDict_C>
                    (OWNER_ADDR);

            if(rto_dict->p_buffer == nullptr) {
                rto_dict->p_buffer = new decltype(fcnmsg::test_ServoPubSubDict)
                        ::Buffer;
                cout << "---> create a buffer!" << endl;
            }
        }
        virtual void spin(){ }

    protected:
        decltype(fcnmsg::test_ServoPubSubDict)* rto_dict;
    };



    class Node07 : public Node{
    public:
        Node07():Node(){}

        void spin() override {
            auto msg = fcnmsg::test_ServoPubSubDict.angle;
            msg << servo_angle;
            servo_angle += 30;
            rto_dict->write(msg);
        }

        int servo_angle{200};
    };

    class Node02 : public Node{
    public:
        Node02():Node(){}
        void spin() override {
            auto angle = rto_dict->read(fcnmsg::test_ServoPubSubDict.angle).data;

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

#endif
#if 1

/*
 * 测试实时对象字典使用网络进行传输
 **/
#include "libfcn/NetworkLayer.hpp"

#include "utils/os_only/HostSerial.hpp"
#include "utils/Tracer.hpp"

#include "ServoPubMsg.hpp"
#include "SimpleSerialNode.hpp"

using namespace libfcn_v2;
using namespace utils;
using namespace fcnmsg;

namespace pubsub_test {

    #define SERVO_ADDR 0x02
    #define DCS_ADDR   0x04
    #define HOST_ADDR  0x05

    FCN_SUBSCRIBE_CALLBACK(servo_speed_cb) {
        LOGD("servo_speed_cb: %d", subscriber->readBuffer(ServoPubMsgOut.speed).data);
    }

    TEST(PubSub, IntraProc) {
        PubSubManager ps_manager(nullptr);
//        auto servo_sub = ps_manager.makeSubscriber(ServoPubMsgIn, SERVO_ADDR, local_addr);

        auto servo_sub_local = ps_manager.makeSubscriber(ServoPubMsgOut, SERVO_ADDR, DCS_ADDR);
//        auto servo_sub_local2 = ps_manager.makeSubscriber(ServoPubMsgOut, SERVO_ADDR, DCS_ADDR);

        servo_sub_local->subscribe(ServoPubMsgOut.speed, servo_speed_cb);

        auto servo_pub = ps_manager.makeMasterPublisher(ServoPubMsgOut, SERVO_ADDR);


        uint32_t cnt = 0;

        for (int __i = 0; __i < 1;) {
            auto speed_msg = ServoPubMsgOut.speed;
            speed_msg << cnt;
            servo_pub->publish(speed_msg);

            auto angle_msg = ServoPubMsgOut.angle;
            angle_msg << cnt;
            servo_pub->publish(angle_msg);

            auto current_msg = ServoPubMsgOut.current;
            current_msg << cnt;
            servo_pub->publish(current_msg);

            perciseSleep(0.1);

            LOGW("servo: speed = %d, angle = %d, current = %d \n",
                 servo_sub_local->readBuffer(ServoPubMsgOut.speed).data,
                 servo_sub_local->readBuffer(ServoPubMsgOut.angle).data,
                 servo_sub_local->readBuffer(ServoPubMsgOut.current).data);

            cnt++;
        }
    }


    TEST(PubSub, Network){
        Node fcn_node(1);

        auto servo_pub = fcn_node.getPubSubManager().
                makeMasterPublisher(ServoPubMsgOut, SERVO_ADDR);
        servo_pub->addPort(0).addPort(1);

        fcn_node.spin();

        uint32_t cnt = 0;

        for (int __i = 0; __i < 1;) {
            auto speed_msg = ServoPubMsgOut.speed;
            speed_msg << cnt;
            servo_pub->publish(speed_msg);

            auto angle_msg = ServoPubMsgOut.angle;
            angle_msg << cnt;
            servo_pub->publish(angle_msg);

            auto current_msg = ServoPubMsgOut.current;
            current_msg << cnt;
            servo_pub->publish(current_msg);

            perciseSleep(0.1);

            cnt++;
        }
    }


    TEST(PubSub, NetworkHost) {
        Node fcn_node(0);

        auto servo_sub = fcn_node.network_layer->pub_sub_manager
                .makeSubscriber(ServoPubMsgOut, SERVO_ADDR, HOST_ADDR);


        fcn_node.spin();

        for(int __i = 0; __i < 1; ){
            perciseSleep(0.1);

            LOGW("servo: speed = %d, angle = %d, current = %d \n",
                 servo_sub->readBuffer(ServoPubMsgOut.speed).data,
                 servo_sub->readBuffer(ServoPubMsgOut.angle).data,
                 servo_sub->readBuffer(ServoPubMsgOut.current).data);
        }

        fcn_node.join();

    }
}

#endif //0
