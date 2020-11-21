//
// Created by sdong on 2020/11/8.
//
#include "libfcn/Node.hpp"
#include "utils/os_only/HostSerial.hpp"
#include "../idl_gen/ServoPubMsg.hpp"

using namespace libfcn_v2;
using namespace utils;
using namespace fcnmsg;

int main(){
    Node node;
    HostSerial serial(1);
    ByteFrameIODevice serial_frame_dev(&serial);

    node.addPort(&serial_frame_dev);

    auto pub_man = node.getPublisherManager();

    auto servo_pub_channel = pub_man->createChannel(ServoPubMsgIn, 0x07);

    auto servo_pub_channel_2 = pub_man->createChannel(ServoPubMsgIn, 0x07);

    int cnt = 0;
    for(int i = 0; i < 1; i){
        auto angle = ServoPubMsgIn.angle;

        angle << cnt;

        servo_pub_channel->publish(angle);

        LOGD("publish a message!");

        LOGD("target_angle = %d", servo_pub_channel->readBuffer(ServoPubMsgIn.target_angle).data);

        LOGD("angle from local node = %d", servo_pub_channel_2->readBuffer(ServoPubMsgIn.target_angle).data);


        perciseSleep(0.5);

        cnt ++;
    }

    node.spin();

    return 0;
}