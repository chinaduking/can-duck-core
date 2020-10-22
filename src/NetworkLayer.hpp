//
// Created by sdong on 2020/10/21.
//

#ifndef LIBFCN_V2_NETWORKLAYER_HPP
#define LIBFCN_V2_NETWORKLAYER_HPP

#include "utils/vector_s.hpp"
#include "DataLinkLayer.hpp"
#include "RealtimeObject.hpp"

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

    private:
        NetworkLayer():
            data_link_dev(4),
            rto_network_handler(
                    RtoShmManager::getInstance(),
                    1000){ //TODO: ferq ctrl??
        }

        static NetworkLayer* instance;



        void recvDispatcher(DataLinkFrame* frame);

    };
}




#endif //LIBFCN_V2_NETWORKLAYER_HPP
