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
    getTracer().setFilter(LogLvl::lDebug);

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
    int16_t target_angle = 0;
    int16_t motor_angle = 0;
    while (1){
        servo_sub->readBuffer(ServoMsgRx.target_angle) >> target_angle;
        LOGD("target_angle = %d", servo_sub->readBuffer(ServoMsgRx.target_angle).data);

        motor_angle = motor_angle * 0.9 + target_angle * 0.1;
        servo_pub->publish(ServoMsgTx.angle(motor_angle));
        emlib::perciseSleep(0.05);
    }
//
//    auto server = ctx.srv().makeServer(ServoSrv, SERVO_ADDR);
//    auto client = ctx.srv().bindServer(ServoSrv, SERVO_ADDR, ECU_ADDR);
}

TEST(Node, EcuMsg){
    getTracer().setFilter(LogLvl::lDebug);

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

    int cmd_state_cnt = 0;
    int cmd_state = 1;
    int16_t target_angle = 0;

    for(int i = 0; i < 1; ){
        target_angle = cmd_state * 0x8FF;

        servo_pub->publish(ServoMsgRx.target_angle(target_angle));
        emlib::perciseSleep(0.05);

        LOGD("target_angle=%d, angle=%d", target_angle,
             servo_sub->readBuffer(ServoMsgTx.angle).data);

        cmd_state_cnt ++;
        if(cmd_state_cnt > 20){
            cmd_state_cnt = 0;
            cmd_state *= -1;
        }
    }
//
//    auto server = ctx.srv().makeServer(ServoSrv, SERVO_ADDR);
//    auto client = ctx.srv().bindServer(ServoSrv, SERVO_ADDR, ECU_ADDR);
}

