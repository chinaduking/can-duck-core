//
// Created by 董世谦 on 2021/3/13.
//

#include "gtest/gtest.h"
#include "Context.hpp"
#include "ServoDict.hpp"
#include "SimCan.hpp"
#include "HostSerial.hpp"

#define SERVO_ADDR 0x02
#define ECU_ADDR   0x05

#define SERVO_SIMCAN_PORT 0
#define ECU_SIMCAN_PORT 1

#define SIMCAN_BAUD 921600

using namespace duckmsg;

TEST(Node, ServoMsg){
    emlib::SimCan can(
        new emlib::HostSerial(SERVO_SIMCAN_PORT, SIMCAN_BAUD)
        );

    can_duck::Context ctx(&can);

    /* Servo Side Init */
    can_duck::Publisher*  servo_pub;
    can_duck::Subscriber* servo_sub;

    std::tie(servo_pub, servo_sub) =
            ctx.msg().bindChannel(
                    ServoMsgTx, ServoMsgRx,
                    SERVO_ADDR,
                    true
            );

    while (1){
        DICT_T(ServoMsgRx.target_angle) target_angle;
        servo_sub->readBuffer(ServoMsgRx.target_angle) >> target_angle;
        LOGD("target_angle = %d", target_angle);

        servo_pub->publish(ServoMsgTx.angle(target_angle * 1.5f));
        emlib::perciseSleep(0.1);
    }
//
//    auto server = ctx.srv().makeServer(ServoSrv, SERVO_ADDR);
//    auto client = ctx.srv().bindServer(ServoSrv, SERVO_ADDR, ECU_ADDR);
}

TEST(Node, EcuMsg){
    emlib::SimCan can(
            new emlib::HostSerial(ECU_SIMCAN_PORT, SIMCAN_BAUD)
    );

    can_duck::Context ctx(&can);

    /* Servo Side Init */
    can_duck::Publisher*  servo_pub;
    can_duck::Subscriber* servo_sub;

    std::tie(servo_pub, servo_sub) =
            ctx.msg().bindChannel(
                    ServoMsgTx, ServoMsgRx,
                    SERVO_ADDR,
                    false
            );

    while (1){
        servo_pub->publish(ServoMsgRx.target_angle(200));
        emlib::perciseSleep(0.1);
    }
//
//    auto server = ctx.srv().makeServer(ServoSrv, SERVO_ADDR);
//    auto client = ctx.srv().bindServer(ServoSrv, SERVO_ADDR, ECU_ADDR);
}

