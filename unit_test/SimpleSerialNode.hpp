//
// Created by sdong on 2020/10/27.
//
#pragma once
#ifndef can_duck_SIMPLESERIALNODE_HPP
#define can_duck_SIMPLESERIALNODE_HPP

#include "NetworkLayer.hpp"
#include "SimCan.hpp"
#include "HostSerial.hpp"
#include "Tracer.hpp"

;

class Node {
public:
    Node(int sid) {
        serial = new emlib::HostSerial(sid);
        can = new emlib::SimCan(serial);

        network_layer = new can_duck::NetworkLayer(can);
    }

    void spin() {
        recv_thread = std::make_shared<std::thread>([&](){
            for (int __i = 0 ; __i < 1; ){
                network_layer->recvPolling();
            }
        });

        send_thread = std::make_shared<std::thread>([&](){
            for (int __i = 0 ; __i < 1; ){
                network_layer->sendPolling();
            }
        });
    }

    void join(){
        if(recv_thread != nullptr){
            recv_thread->join();
        }
    }

    can_duck::PubSubManager& getPubSubManager(){
        return network_layer->pub_sub_manager;
    }

    emlib::HostSerial* serial;
    LLCanBus* can;
    can_duck::NetworkLayer* network_layer;

    std::shared_ptr<std::thread> recv_thread  {nullptr};
    std::shared_ptr<std::thread> send_thread  {nullptr};
};

#endif //can_duck_SIMPLESERIALNODE_HPP