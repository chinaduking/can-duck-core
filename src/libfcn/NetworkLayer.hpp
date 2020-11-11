//
// Created by sdong on 2020/10/21.
//

#ifndef LIBFCN_V2_NETWORKLAYER_HPP
#define LIBFCN_V2_NETWORKLAYER_HPP

#include "vector_s.hpp"
#include "DataLinkLayer.hpp"
#include "PubSub.hpp"
#include "ParamServer.hpp"
#include "DefaultAllocate.h"
#include "utils/Tracer.hpp"

namespace libfcn_v2{
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

        PublisherManager   pub_sub_manager;
        ParamServerManager param_server_manager;
        //LargeDataHandler large_data_handler;

    private:
        utils::vector_s<FrameIODevice*> data_link_dev;
        void recvProtocolDispatcher(FcnFrame* frame, uint16_t recv_port_id);
    };
}




#endif //LIBFCN_V2_NETWORKLAYER_HPP
