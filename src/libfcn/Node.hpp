//
// Created by sdong on 2020/10/31.
//

#ifndef LIBFCN_V2_NODE_HPP
#define LIBFCN_V2_NODE_HPP

#include "DataLinkLayer.hpp"
#include "PubSub.hpp"
#include "ParamServer.hpp"

#ifdef SYSTYPE_FULL_OS
    #include <vector>
    #include <thread>
    #include <mutex>
#endif //SYSTYPE_FULL_OS

namespace libfcn_v2{

    class NetworkLayer;

    class Node {
    public:
        Node();
        virtual ~Node() = default;

        PublisherManager* getPublisherManager();
        ParamServerManager* getParamServerManager();
        int addPort(FrameIODevice* device);

#ifdef SYSTYPE_FULL_OS
        void stop();
        void spin();
#endif //SYSTYPE_FULL_OS

        void spinOnce();

    protected:
        static NetworkLayer* network_layer;

#ifdef SYSTYPE_FULL_OS
        std::vector<std::thread> send_threads;
        std::vector<std::thread> recv_threads;
        std::mutex io_mutex;
#endif //SYSTYPE_FULL_OS

        void sendPolling();
        void recvPolling();
    };

}

#endif //LIBFCN_V2_NODE_HPP
