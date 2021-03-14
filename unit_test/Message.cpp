//
// Created by sdong on 2020/10/15.
//

#include "TestUtils.hpp"
#include "Message.hpp"
#include "ServoDict.hpp"
using namespace can_duck;



/*
 * 测试实时对象字典使用网络进行传输
 **/
#include "HostSerial.hpp"
#include "Tracer.hpp"
#include "ServoDict.hpp"
#include "SimCan.hpp"
#include "Common.hpp"

using namespace can_duck;
using namespace emlib;
using namespace duckmsg;

namespace pubsub_test {

    #define SERVO_ADDR 0x02
    #define ECU_ADDR   0x04
    #define HOST_ADDR  0x05

    DUCK_SUBSCRIBE_CALLBACK(servo_speed_cb) {
        LOGD("servo_speed_cb: %d", subscriber->readBuffer(ServoMsgTx.speed).data);
    }

    TEST(PubSub, InProc) {
        MessageContext ps_manager(nullptr);

        /* Servo Side Init */
        Publisher*  pub_servo_to_any;
        Subscriber* sub_any_to_servo;

        std::tie(pub_servo_to_any, sub_any_to_servo) =
                ps_manager.bindChannel(ServoMsgTx, ServoMsgRx,
                                       SERVO_ADDR, true);

        /* ECU Side Init */
        Publisher*  pub_ecu_to_servo;
        Subscriber* sub_servo_to_ecu;

        std::tie(pub_ecu_to_servo, sub_servo_to_ecu) =
                ps_manager.bindChannel(ServoMsgTx, ServoMsgRx,
                                       SERVO_ADDR, false);

        sub_servo_to_ecu->subscribe(ServoMsgTx.speed, servo_speed_cb);



        uint32_t cnt = 0;
        for (int __i = 0; __i < 1;) {
            /* Servo Side Run */
            auto speed_msg = ServoMsgTx.speed;
            speed_msg << cnt;
            pub_servo_to_any->publish(speed_msg);

            auto angle_msg = ServoMsgTx.angle;
            angle_msg << cnt;
            pub_servo_to_any->publish(angle_msg);

            auto current_msg = ServoMsgTx.current;
            current_msg << cnt;
            pub_servo_to_any->publish(current_msg);

            /* ECU Side Run */
            LOGW("servo: speed = %d, angle = %d, current = %d \n",
                 sub_servo_to_ecu->readBuffer(ServoMsgTx.speed).data,
                 sub_servo_to_ecu->readBuffer(ServoMsgTx.angle).data,
                 sub_servo_to_ecu->readBuffer(ServoMsgTx.current).data);

            perciseSleep(0.1);

            cnt++;
        }
    }

    class PubNode {
    public:
        PubNode(int sid){
            can = new emlib::SimCan(new emlib::HostSerial(sid));
            pubsub = new can_duck::MessageContext(can);
        }

        void spin() {
            recv_thread = std::make_shared<std::thread>([&](){
                CANMessage rx_msg;
                for (int __i = 0 ; __i < 1; ){
                    if(can->read(rx_msg)){
                        pubsub->__handleRecv(&rx_msg, 0);
                    }
                }});
        }

        void join(){ recv_thread->join(); }

        LLCanBus* can;
        can_duck::MessageContext* pubsub;
        std::shared_ptr<std::thread> recv_thread  {nullptr};
    };

    TEST(PubSub, Network){
        PubNode fcn_node(1);

        Publisher*  servo_pub;
        Subscriber* servo_sub;

        std::tie(servo_pub, servo_sub) = fcn_node.pubsub
                ->bindChannel(ServoMsgTx, ServoMsgRx, SERVO_ADDR, true);

        fcn_node.spin();

        uint32_t cnt = 0;

        for (int __i = 0; __i < 1;) {
            auto speed_msg = ServoMsgTx.speed;
            speed_msg << cnt;
            servo_pub->publish(speed_msg);

            auto angle_msg = ServoMsgTx.angle;
            angle_msg << cnt;
            servo_pub->publish(angle_msg);

            auto current_msg = ServoMsgTx.current;
            current_msg << cnt;
            servo_pub->publish(current_msg);

            perciseSleep(0.1);

            cnt++;
        }
    }


    TEST(PubSub, NetworkHost) {
        PubNode fcn_node(0);

        Publisher*  servo_pub;
        Subscriber* servo_sub;

        std::tie(servo_pub, servo_sub) = fcn_node.pubsub
                ->bindChannel(ServoMsgTx, ServoMsgRx, SERVO_ADDR, false);


        fcn_node.spin();

        for(int __i = 0; __i < 1; ){
            perciseSleep(0.1);

            LOGW("servo: speed = %d, angle = %d, current = %d \n",
                 servo_sub->readBuffer(ServoMsgTx.speed).data,
                 servo_sub->readBuffer(ServoMsgTx.angle).data,
                 servo_sub->readBuffer(ServoMsgTx.current).data);
        }

        fcn_node.join();

    }
}

