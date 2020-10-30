//
// Created by sdong on 2020/10/27.
//

#include "TestUtils.hpp"
#include "NetworkLayer.hpp"

#include "utils/PosixSerial.hpp"
#include "utils/Tracer.hpp"

#include "test_ServoRTO.hpp"
#include "FrameUtils.hpp"
#include "SimpleSerialNode.hpp"

using namespace libfcn_v2;
using namespace utils;

namespace network_test {

    #define SERVO_ADDR 0x02

    TEST(NetworkLayer, SvoServoNode) {
        int local_addr = SERVO_ADDR;

        Node fcn_node(0);

        DataLinkFrame frame_tmp;


        fcn_node.spin();

        uint32_t cnt = 0;

        auto server = fcn_node.network_layer->svo_network_handler
                .createServer(fcnmsg::test_ServoRTO, local_addr);

        for(int __i = 0; __i < 1; ){
            auto angle_msg = fcnmsg::test_ServoRTO.angle;
            angle_msg << cnt;
            server->updateData(angle_msg);

            cnt ++;

            perciseSleep(0.5);
        }
        fcn_node.join();
    }


    TEST(NetworkLayer, SvoHostNode) {
        Node fcn_node(1);
        Tracer tracer(true);
        tracer.setFilter(Tracer::INFO);

        int servo_addr = SERVO_ADDR;

        fcn_node.spin();


        auto servo_client = fcn_node.network_layer->svo_network_handler
                .bindClienttoServer(servo_addr);

        for(int __i = 0; __i < 1; ){
            perciseSleep(0.1);

            servo_client->readUnblocking(fcnmsg::test_ServoRTO.angle);

//            tracer.print(Tracer::WARNING, "servo: speed = %d, angle = %d"
//                                          ", current = %d \n",
//                         servo_rto_dict->speed.data,
//                         servo_rto_dict->angle.data,
//                         servo_rto_dict->current.data);
        }

        fcn_node.join();
    }

}