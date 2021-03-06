//
// Created by sdong on 2020/10/27.
//

#include "TestUtils.hpp"
#include "ServoDict.hpp"
#include "Service.hpp"
#include "SimCan.hpp"
#include "HostSerial.hpp"
#include "Tracer.hpp"

using namespace can_duck;
using namespace emlib;
using namespace duckmsg;

namespace service_test {

    #define SERVO_ADDR 0x02
    #define HOST_ADDR  0x05

    TEST(nullptr, test)  {
        ASSERT_EQ(std::unique_ptr<int>(nullptr), nullptr);
    }

    TEST(callback, test) {
        cout << sizeof(std::function<int(int)>) << endl;
    }

    class SrvNode {
    public:
        SrvNode(int sid){
            can = new emlib::SimCan(new emlib::HostSerial(sid));
            srv = new can_duck::ServiceContext(can);
        }

        void spin() {
            recv_thread = std::make_shared<std::thread>([&](){
                CANMessage rx_msg;
                for (int __i = 0 ; __i < 1; ){
                    if(can->read(rx_msg)){
                        srv->__handleRecv(&rx_msg, 0);
                    }
                }});
        }

        void join(){ recv_thread->join(); }

        LLCanBus* can;
        can_duck::ServiceContext* srv;
        std::shared_ptr<std::thread> recv_thread  {nullptr};
    };


    TEST(ParamServer, Server) {
        int local_addr = SERVO_ADDR;

        SrvNode fcn_node(0);

        uint32_t cnt = 0;

        auto server = fcn_node.srv
                ->makeServer(ServoSrv, local_addr);

        server->setWrAccess(ServoSrv.mode);

        fcn_node.spin();

        for(int __i = 0; __i < 1; ){
            auto mode_msg = ServoSrv.mode;
            mode_msg << 0x02;
            server->updateData(mode_msg);

            cnt ++;
            cout << "server->updateData(angle_msg): " << cnt << endl;

            perciseSleep(0.5);
        }
        fcn_node.join();
    }

    DUCK_REQUEST_CALLBACK(angle_rd_callback){
        if(ev_code == 2){
            LOGE("Read angle timeout");
            return;
        }

        if(ev_code == 1){
            auto angle = client->readBuffer(ServoSrv.mode).data;

            LOGW("read angle done: 0x%X", angle);
            return;
        }

    }

    DUCK_REQUEST_CALLBACK(mode_wr_callback){
        if(ev_code == 2){
            LOGE("Write angle timeout");
            return;
        }

        if(ev_code == 1){
            LOGW("Write mode done!");
            return;

        }

        LOGE("Write angle failed");

    }


    TEST(ParamServer, Client) {
        SrvNode fcn_node(1);

        int servo_addr = SERVO_ADDR;
        int local_addr = HOST_ADDR;

        fcn_node.spin();

        auto servo_client = fcn_node.srv->
                bindServer(ServoSrv, servo_addr, local_addr, 0);

        for(int __i = 0; __i < 1; ){
            LOGD("request.. " );

            servo_client->readAsync(ServoSrv.mode, angle_rd_callback);

            servo_client->readAsync(ServoSrv.mode, angle_rd_callback);

            auto mode_msg = ServoSrv.mode;
            mode_msg << 0x22;

            servo_client->writeAsync(mode_msg, mode_wr_callback);

            perciseSleep(1);
        }

        fcn_node.join();
    }

}