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

        static NetworkLayer* getInstance();

        ~NetworkLayer() = default;

        int addDataLinkDevice(FrameIODevice* device);

        //bool sendFrame(uint16_t port_id, DataLinkFrame* frame);

        void recvPolling();
        void sendPolling();

        utils::vector_s<FrameIODevice*> data_link_dev;
        RtoNetworkHandler rto_network_handler;

        utils::vector_s<SvoServer*> svo_server_local;
        utils::vector_s<SvoServer*> svo_client_local;


    private:
        NetworkLayer():
            data_link_dev(4),
            svo_server_local(MAX_LOCAL_NODE),
            svo_client_local(MAX_LOCAL_NODE),
            rto_network_handler(1000)//TODO: ferq ctrl??
        {}

        static NetworkLayer* instance;



        void recvDispatcher(DataLinkFrame* frame, uint16_t recv_port_id);

    };
}




#endif //LIBFCN_V2_NETWORKLAYER_HPP
