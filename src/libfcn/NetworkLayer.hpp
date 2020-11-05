//
// Created by sdong on 2020/10/21.
//

#ifndef LIBFCN_V2_NETWORKLAYER_HPP
#define LIBFCN_V2_NETWORKLAYER_HPP

#include "utils/vector_s.hpp"
#include "DataLinkLayer.hpp"
#include "PubSub.hpp"
#include "ParamServer.hpp"
#include "DefaultAllocate.h"
#include "Log.hpp"

namespace libfcn_v2{
    class NetworkLayer {
    public:
        NetworkLayer()
            :
            rto_network_handler(this),
            svo_network_handler(this),
            data_link_dev(MAX_COM_PORT_NUM) {}

        ~NetworkLayer() = default;

        int addDataLinkDevice(FrameIODevice* device);

        int sendFrame(uint16_t port_id, DataLinkFrame* frame);

        void recvPolling();
        void sendPolling();


        PubNetworkHandler rto_network_handler;
        ParamServerNetHandle svo_network_handler;
        //LargeDataHandler large_data_handler;


    private:
        utils::vector_s<FrameIODevice*> data_link_dev;
        void recvProtocolDispatcher(DataLinkFrame* frame, uint16_t recv_port_id);
    };
}




#endif //LIBFCN_V2_NETWORKLAYER_HPP
