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

using namespace duckmsg;

TEST(Node, msg){
    emlib::SimCan can(
        new emlib::HostSerial(0, 921600)
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

    servo_pub->publish(ServoMsgTx.angle(200));

    auto server = ctx.srv().makeServer(ServoSrv, SERVO_ADDR);
    auto client = ctx.srv().bindServer(ServoSrv, SERVO_ADDR, ECU_ADDR);
}
