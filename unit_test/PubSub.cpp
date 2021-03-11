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
    #define ECU_ADDR   0x04
    #define HOST_ADDR  0x05


    FCN_SUBSCRIBE_CALLBACK(servo_speed_cb) {
        LOGD("servo_speed_cb: %d", subscriber->readBuffer(servo_msg_o.speed).data);
    }

    TEST(PubSub, IntraProc) {
        PubSubManager ps_manager(nullptr);

        /* Servo Side Init */
        Publisher*  pub_servo_to_any;
        Subscriber* sub_any_to_servo;

        std::tie(pub_servo_to_any, sub_any_to_servo) =
                ps_manager.bindPubChannel(servo_msg_o, servo_msg_i,
                                  SERVO_ADDR, true);

        /* ECU Side Init */
        Publisher*  pub_ecu_to_servo;
        Subscriber* sub_servo_to_ecu;

        std::tie(pub_ecu_to_servo, sub_servo_to_ecu) =
                ps_manager.bindPubChannel(servo_msg_o, servo_msg_i,
                                  SERVO_ADDR, false);

        sub_servo_to_ecu->subscribe(servo_msg_o.speed, servo_speed_cb);



        uint32_t cnt = 0;
        for (int __i = 0; __i < 1;) {
            /* Servo Side Run */
            auto speed_msg = servo_msg_o.speed;
            speed_msg << cnt;
            pub_servo_to_any->publish(speed_msg);

            auto angle_msg = servo_msg_o.angle;
            angle_msg << cnt;
            pub_servo_to_any->publish(angle_msg);

            auto current_msg = servo_msg_o.current;
            current_msg << cnt;
            pub_servo_to_any->publish(current_msg);

            /* ECU Side Run */
            LOGW("servo: speed = %d, angle = %d, current = %d \n",
                 sub_servo_to_ecu->readBuffer(servo_msg_o.speed).data,
                 sub_servo_to_ecu->readBuffer(servo_msg_o.angle).data,
                 sub_servo_to_ecu->readBuffer(servo_msg_o.current).data);

            perciseSleep(0.1);

            cnt++;
        }
    }

#if 0
    TEST(PubSub, Network){
        Node fcn_node(1);

        auto servo_pub = fcn_node.getPubSubManager().
                makePublisher(servo_msg_o, SERVO_ADDR);
        servo_pub->addPort(0).addPort(1);

        fcn_node.spin();

        uint32_t cnt = 0;

        for (int __i = 0; __i < 1;) {
            auto speed_msg = servo_msg_o.speed;
            speed_msg << cnt;
            servo_pub->publish(speed_msg);

            auto angle_msg = servo_msg_o.angle;
            angle_msg << cnt;
            servo_pub->publish(angle_msg);

            auto current_msg = servo_msg_o.current;
            current_msg << cnt;
            servo_pub->publish(current_msg);

            perciseSleep(0.1);

            cnt++;
        }
    }


    TEST(PubSub, NetworkHost) {
        Node fcn_node(0);

        auto servo_sub = fcn_node.network_layer->pub_sub_manager
                .makeSubscriber(servo_msg_o, SERVO_ADDR, HOST_ADDR);


        fcn_node.spin();

        for(int __i = 0; __i < 1; ){
            perciseSleep(0.1);

            LOGW("servo: speed = %d, angle = %d, current = %d \n",
                 servo_sub->readBuffer(servo_msg_o.speed).data,
                 servo_sub->readBuffer(servo_msg_o.angle).data,
                 servo_sub->readBuffer(servo_msg_o.current).data);
        }

        fcn_node.join();

    }
#endif //0
}

