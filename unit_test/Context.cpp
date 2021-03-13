//
// Created by 董世谦 on 2021/3/13.
//

#include "gtest/gtest.h"
#include "Context.hpp"
#include "ServoDict.hpp"
#include "SimCan.hpp"
#include "HostSerial.hpp"

#define SERVO_ADDR 0x02

TEST(Node, msg){
    emlib::SimCan can(
        new emlib::HostSerial(0, 921600)
        );

    can_duck::Context ctx(&can);
    /* Servo Side Init */
    can_duck::Publisher*  pub_servo_to_any;
    can_duck::Subscriber* sub_any_to_servo;

    std::tie(pub_servo_to_any, sub_any_to_servo) =
            ctx.msg().bindMessageChannel(
                    duckmsg::servo_msg_o, duckmsg::servo_msg_i,
                    SERVO_ADDR,
                    true
                    );
}
