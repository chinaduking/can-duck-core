//
// Created by sdong on 2020/10/21.
//

#ifndef can_duck_NETWORKLAYER_HPP
#define can_duck_NETWORKLAYER_HPP

//#include "DataLinkLayer.hpp"
#include "PubSub.hpp"
#include "ParamServer.hpp"
#include "DefaultAllocate.h"
#include "Vector.hpp"
#include "Tracer.hpp"

namespace can_duck{
    class NetworkLayer {
    public:
        NetworkLayer(LLCanBus* can)
            :
                pub_sub_manager(this)
                ,param_server_manager(can),
                can_bus(can)
                {}

        ~NetworkLayer() = default;

//        int addDataLinkDevice(LLCanBus* device);

        int sendFrame(uint16_t port_id, CANMessage* frame);

        void recvPolling();
        void sendPolling();

        PubSubManager   pub_sub_manager;
        ParamServerManager param_server_manager;
        //TftpManager tftp_handler;

    private:
        LLCanBus* const can_bus{nullptr};
    };
}




#endif //can_duck_NETWORKLAYER_HPP
