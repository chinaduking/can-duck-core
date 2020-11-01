//
// Created by sdong on 2020/10/27.
//

#include "TestUtils.hpp"
#include "NetworkLayer.hpp"

#include "utils/PosixSerial.hpp"
#include "utils/Tracer.hpp"

#include "test_ServoDict.hpp"
#include "SimpleSerialNode.hpp"

using namespace libfcn_v2;
using namespace utils;

namespace network_test {

    #define SERVO_ADDR 0x02
    #define HOST_ADDR  0x05

    TEST(nullptr, test){
        ASSERT_EQ(std::unique_ptr<int>(nullptr), nullptr);
    }

    TEST(NetworkLayer, SvoServoNode) {
        int local_addr = SERVO_ADDR;

        Node fcn_node(0);

        DataLinkFrame frame_tmp;

        uint32_t cnt = 0;

        auto server = fcn_node.network_layer->svo_network_handler
                .createServer(fcnmsg::test_ServoPubSubDict, local_addr);

        server->setWrAccess(fcnmsg::test_ServoPubSubDict.mode);

        fcn_node.spin();

        for(int __i = 0; __i < 1; ){
            auto angle_msg = fcnmsg::test_ServoPubSubDict.angle;
            angle_msg << 0x55667788;
            server->updateData(angle_msg);

            cnt ++;
            cout << "server->updateData(angle_msg): " << cnt << endl;

            perciseSleep(0.5);
        }
        fcn_node.join();
    }


    TEST(NetworkLayer, SvoHostNode) {
        Node fcn_node(1);
        Tracer tracer(true);
        tracer.setFilter(Tracer::Level::INFO);

        int servo_addr = SERVO_ADDR;
        int local_addr = HOST_ADDR;

        fcn_node.spin();


        auto servo_client = fcn_node.network_layer->svo_network_handler
                .bindClientToServer(servo_addr, local_addr, 0);



        for(int __i = 0; __i < 1; ){
            perciseSleep(1);

            cout << "request.. "  << endl;

            servo_client->readUnblocking(fcnmsg::test_ServoPubSubDict.angle);

//            auto mode_msg = fcnmsg::test_ServoPubSubDict.mode;
//            mode_msg << 0x22;
//            servo_client->writeUnblocking(mode_msg);

//            tracer.print(Tracer::WARNING, "servo: speed = %d, angle = %d"
//                                          ", current = %d \n",
//                         servo_rto_dict->speed.data,
//                         servo_rto_dict->angle.data,
//                         servo_rto_dict->current.data);
        }

        fcn_node.join();
    }

}