//
// Created by sdong on 2020/10/21.
//

#ifndef can_duck_NETWORKLAYER_HPP
#define can_duck_NETWORKLAYER_HPP

#include "DataLinkLayer.hpp"
#include "PubSub.hpp"
#include "ParamServer.hpp"
#include "DefaultAllocate.h"
#include "Vector.hpp"
#include "Tracer.hpp"

namespace can_duck{
    class NetworkLayer {
    public:
        NetworkLayer()
            :
                pub_sub_manager(this),
                param_server_manager(this),
                data_link_dev(MAX_COM_PORT_NUM) {}

        ~NetworkLayer() = default;

        int addDataLinkDevice(FrameIODevice* device);

        int sendFrame(uint16_t port_id, FcnFrame* frame);

        void recvPolling();
        void sendPolling();

        PubSubManager   pub_sub_manager;
        ParamServerManager param_server_manager;
        //LargeDataHandler large_data_handler;

    private:
        emlib::Vector<FrameIODevice*> data_link_dev;
        void recvProtocolDispatcher(FcnFrame* frame, uint16_t recv_port_id);
    };
}




#endif //can_duck_NETWORKLAYER_HPP
