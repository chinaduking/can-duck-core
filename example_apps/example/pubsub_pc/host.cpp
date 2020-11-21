//
// Created by sdong on 2020/10/22.
//

#include "libfcn/Node.hpp"
#include "utils/os_only/HostSerial.hpp"
#include "../idl_gen/ServoPubMsg.hpp"

using namespace libfcn_v2;
using namespace utils;
using namespace fcnmsg;

int main(){
    Node node;
    HostSerial serial(0);
    ByteFrameIODevice serial_frame_dev(&serial);

    node.addPort(&serial_frame_dev);

    auto pub_man = node.getPublisherManager();

    auto servo_pub_channel = pub_man->createChannel(ServoPubMsgIn, 0x07);

    int cnt = 0;

    for(int i = 0; i < 1; i){
        auto target_angle = ServoPubMsgIn.target_angle;

        target_angle  << cnt;

        servo_pub_channel->publish(target_angle);

        LOGD("publish a message!");

        LOGD("angle = %d", servo_pub_channel->readBuffer(ServoPubMsgIn.angle).data);

        perciseSleep(0.5);
        cnt ++;
    }

//    node.spin();

    return 0;
}