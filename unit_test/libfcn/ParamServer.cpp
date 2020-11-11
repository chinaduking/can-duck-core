//
// Created by sdong on 2020/10/27.
//

#include "TestUtils.hpp"
#include "libfcn/NetworkLayer.hpp"

#include "utils/os_only/HostSerial.hpp"
#include "utils/Tracer.hpp"

#include "test_ServoDict.hpp"
#include "SimpleSerialNode.hpp"

using namespace libfcn_v2;
using namespace utils;
using namespace fcnmsg;

namespace network_test {

    #define SERVO_ADDR 0x02
    #define HOST_ADDR  0x05

    TEST(nullptr, test)  {
        ASSERT_EQ(std::unique_ptr<int>(nullptr), nullptr);
    }

    TEST(callback, test) {
        cout << sizeof(std::function<int(int)>) << endl;
    }


    TEST(NetworkLayer, SvoServoNode) {
        int local_addr = SERVO_ADDR;

        Node fcn_node(0);

        FcnFrame frame_tmp;

        uint32_t cnt = 0;

        auto server = fcn_node.network_layer->param_server_manager
                .createServer(test_ServoPubSubDict, local_addr);

        server->setWrAccess(test_ServoPubSubDict.mode);

        fcn_node.spin();

        for(int __i = 0; __i < 1; ){
            auto angle_msg = test_ServoPubSubDict.angle;
            angle_msg << 0x55667788;
            server->updateData(angle_msg);

            cnt ++;
            cout << "server->updateData(angle_msg): " << cnt << endl;

            perciseSleep(0.5);
        }
        fcn_node.join();
    }


    void angle_rd_callback(void* obj_ptr, int ev_code, FcnFrame* frame){
        if(ev_code == 2){
            LOGW("Read angle timeout");
            return;
        }

        if(ev_code == 1){
            auto angle = ParamServerClient::readBuffer(
                    test_ServoPubSubDict.angle, frame).data;

            LOGD("read angle done: 0x%X", angle);
        }
    }

    void mode_wr_callback(void* obj_ptr, int ev_code, FcnFrame* frame){
        if(ev_code == 1){
            LOGW("Write mode done!");
        }
        else{
            LOGW("Write mode failed!");
        }
    }

    TEST(NetworkLayer, SvoHostNode) {
        Node fcn_node(1);

        int servo_addr = SERVO_ADDR;
        int local_addr = HOST_ADDR;

        fcn_node.spin();


        auto servo_client = fcn_node.network_layer->param_server_manager
                .bindClientToServer(servo_addr, local_addr, 0);

        for(int __i = 0; __i < 1; ){
            LOGD("request.. " );


            servo_client->readUnblocking(test_ServoPubSubDict.angle,
                                         RequestCallback(angle_rd_callback));

            servo_client->readUnblocking(test_ServoPubSubDict.angle,
                                         RequestCallback(angle_rd_callback));

            auto mode_msg = test_ServoPubSubDict.mode;
            mode_msg << 0x22;

            servo_client->writeUnblocking(mode_msg,
                                          RequestCallback(mode_wr_callback));

            perciseSleep(0.1);
        }

        fcn_node.join();
    }

}