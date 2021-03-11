//
// Created by sdong on 2020/10/31.
//

#ifndef can_duck_NODE_HPP
#define can_duck_NODE_HPP

#include "DataLinkLayer.hpp"
#include "PubSub.hpp"
#include "ParamServer.hpp"

#ifdef SYSTYPE_FULL_OS
    #include <vector>
    #include <thread>
    #include <mutex>
#endif //SYSTYPE_FULL_OS

namespace can_duck{

    class NetworkLayer;

    class Node {
    public:
        Node();
        ~Node(){
#ifdef SYSTYPE_FULL_OS
            stop();
            spin();
#endif //SYSTYPE_FULL_OS

        }

        int addPort(FrameIODevice* device);

        PubSubManager* getPublisherManager();
        ParamServerManager* getParamServerManager();

#ifdef SYSTYPE_FULL_OS
        void stop();
        void spin();
#endif //SYSTYPE_FULL_OS

        void spinOnce();

    protected:
        static NetworkLayer* network_layer;

#ifdef SYSTYPE_FULL_OS
        std::thread* send_threads{nullptr};
        std::thread* recv_threads{nullptr};
#endif //SYSTYPE_FULL_OS

        void sendPolling();
        void recvPolling();

#ifdef SYSTYPE_FULL_OS
        bool stop_flag{false};
#endif
    };

}

#endif //can_duck_NODE_HPP
