//
// Created by sdong on 2020/10/27.
//

#include "TestUtils.hpp"
#include "NetworkLayer.hpp"

#include "utils/PosixSerial.hpp"
#include "utils/Tracer.hpp"

#include "testServoRtoDict.hpp"
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

        auto rto_dict = fcn_node.network_layer->rto_network_handler.
                bindDictToChannel<libfcn_v2_test::testServoRtoDict>(local_addr);

        fcn_node.spin();

        uint32_t cnt = 0;

        while(1){
            rto_dict->speed << cnt;
            rto_dict->angle << cnt;
            rto_dict->current << cnt;

            RtoFrameBuilder(&frame_tmp, rto_dict,
                            rto_dict->speed.index,
                            rto_dict->current.index);

            frame_tmp.src_id = local_addr;
            frame_tmp.dest_id = 0x00; /*ANY*/

            cout << DataLinkFrameToString(frame_tmp) << endl;

            fcn_node.frame_dev->write(&frame_tmp);

            //fcn_node.spin();
            perciseSleep(0.1);

            cnt ++;
        }
    }


    TEST(NetworkLayer, SvoHostNode) {
        Node fcn_node(1);
        Tracer tracer(true);
        tracer.setFilter(Tracer::INFO);

        int servo_addr = SERVO_ADDR;

        auto servo_rto_dict = fcn_node.network_layer->rto_network_handler.
                bindDictToChannel<libfcn_v2_test::testServoRtoDict>(servo_addr);

        fcn_node.spin();

        while(1){
//            fcn_node.spin();
            perciseSleep(0.1);

            tracer.print(Tracer::WARNING, "servo: speed = %d, angle = %d"
                                          ", current = %d \n",
                         servo_rto_dict->speed.data,
                         servo_rto_dict->angle.data,
                         servo_rto_dict->current.data);
        }

        fcn_node.join();
    }

}