//
// Created by sdong on 2020/10/27.
//
#pragma once
#ifndef LIBFCN_V2_SIMPLESERIALNODE_HPP
#define LIBFCN_V2_SIMPLESERIALNODE_HPP

#include "libfcn/NetworkLayer.hpp"

#include "utils/os_only/HostSerial.hpp"
#include "utils/Tracer.hpp"

namespace network_test{
    libfcn_v2::NetworkLayer gNetworkLayer;

    class Node {
    public:
        Node(int sid) :
                network_layer(&gNetworkLayer) {

            serial = new utils::HostSerial(sid);
            frame_dev = new libfcn_v2::ByteFrameIODevice(serial);
            network_layer->addDataLinkDevice(frame_dev);
        }

        void spin() {
            recv_thread = std::make_shared<std::thread>([&](){
                for (int __i = 0 ; __i < 1; ){
                    network_layer->recvPolling();
                }
            });
        }

        void join(){
            if(recv_thread != nullptr){
                recv_thread->join();
            }
        }

        utils::HostSerial* serial;
        libfcn_v2::ByteFrameIODevice* frame_dev;
        libfcn_v2::NetworkLayer *const network_layer;

        std::shared_ptr<std::thread> recv_thread  {nullptr};
    };

}

#endif //LIBFCN_V2_SIMPLESERIALNODE_HPP
