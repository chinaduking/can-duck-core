//
// Created by sdong on 2020/10/15.
//

#include "TestUtils.hpp"
#include "PubSub.hpp"
#include "ServoPubMsg.hpp"
using namespace libfcn_v2;



/*
 * 测试实时对象字典使用网络进行传输
 **/
#include "NetworkLayer.hpp"

#include "HostSerial.hpp"
#include "Tracer.hpp"

#include "ServoPubMsg.hpp"
#include "SimpleSerialNode.hpp"

using namespace libfcn_v2;
using namespace emlib;
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

        auto pub_servo2any = ps_manager.makePublisher (ServoPubMsgOut, SERVO_ADDR, true);
        auto sub_any2servo = ps_manager.makeSubscriber(ServoPubMsgIn,  SERVO_ADDR, true);

        auto pub_ecu2servo = ps_manager.makePublisher (ServoPubMsgIn,  SERVO_ADDR, false);
        auto sub_servo2ecu = ps_manager.makeSubscriber(ServoPubMsgOut, SERVO_ADDR, false);

        sub_servo2ecu->subscribe(ServoPubMsgOut.speed, servo_speed_cb);

        uint32_t cnt = 0;

        for (int __i = 0; __i < 1;) {
            auto speed_msg = ServoPubMsgOut.speed;
            speed_msg << cnt;
            pub_servo2any->publish(speed_msg);

            auto angle_msg = ServoPubMsgOut.angle;
            angle_msg << cnt;
            pub_servo2any->publish(angle_msg);

            auto current_msg = ServoPubMsgOut.current;
            current_msg << cnt;
            pub_servo2any->publish(current_msg);

            LOGW("servo: speed = %d, angle = %d, current = %d \n",
                 sub_servo2ecu->readBuffer(ServoPubMsgOut.speed).data,
                 sub_servo2ecu->readBuffer(ServoPubMsgOut.angle).data,
                 sub_servo2ecu->readBuffer(ServoPubMsgOut.current).data);

            perciseSleep(0.1);

            cnt++;
        }
    }

#if 0
    TEST(PubSub, Network){
        Node fcn_node(1);

        auto servo_pub = fcn_node.getPubSubManager().
                makePublisher(ServoPubMsgOut, SERVO_ADDR);
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
