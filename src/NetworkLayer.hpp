//
// Created by sdong on 2020/10/21.
//

#ifndef LIBFCN_V2_NETWORKLAYER_HPP
#define LIBFCN_V2_NETWORKLAYER_HPP

#include "utils/vector_s.hpp"
#include "DataLinkLayer.hpp"
#include "RealtimeObject.hpp"
#include "ServiceObject.hpp"
#include "DefaultAllocate.h"

namespace libfcn_v2{
    class NetworkLayer {
    public:
        NetworkLayer()
            :
            data_link_dev(MAX_COM_PORT_NUM),
            rto_network_handler(this),
            svo_network_handler(this)
            {}

        ~NetworkLayer() = default;

        int addDataLinkDevice(FrameIODevice* device);

        //bool sendFrame(uint16_t port_id, DataLinkFrame* frame);

        void recvPolling();
        void sendPolling();

        utils::vector_s<FrameIODevice*> data_link_dev;
        RtoNetworkHandler rto_network_handler;
        SvoNetworkHandler svo_network_handler;
        //LargeDataHandler large_data_handler;


    private:

        void recvDispatcher(DataLinkFrame* frame, uint16_t recv_port_id);

    };
}




#endif //LIBFCN_V2_NETWORKLAYER_HPP
