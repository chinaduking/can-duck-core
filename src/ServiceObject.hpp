//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_SERVICEOBJECT_HPP
#define LIBFCN_V2_SERVICEOBJECT_HPP

#include <cstdint>
#include "utils/vector_s.hpp"
#include "DataLinkLayer.hpp"
#include "DataObjects.hpp"
#include "SharedObjManager.hpp"
#include "DefaultAllocate.h"

namespace libfcn_v2 {

    class NetworkLayer;

    /* 共享字典管理器 */
    typedef SharedObjManager<ServiceObjectDict> SvoDictManager;

    /* 共享字典管理器 */

    class SvoNetworkHandler{
    public:
        SvoNetworkHandler(NetworkLayer* network):
                network(network),
                dict_manager(MAX_NODE_NUM)
        {}


        virtual ~SvoNetworkHandler() = default;

        ServiceObjectDict* bindDictAsServer(uint16_t address);
        ServiceObjectDict* bindDictAsClient(uint16_t address);

        void handleRecv(DataLinkFrame* frame, uint16_t recv_port_id);

        NetworkLayer* network{nullptr};

        uint16_t address{0};

        uint8_t is_server{0};


    protected:
        /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
        static obj_size_t onWriteReq(ServiceObjectDict* dict,
                                     obj_idx_t index,
                                     uint8_t *data, obj_size_t len);

        static void onReadAck(ServiceObjectDict* dict,
                              obj_idx_t index,
                              uint8_t *data, obj_size_t len);

        static void onWriteAck(ServiceObjectDict* dict,
                               obj_idx_t index, uint8_t result);

        SvoDictManager dict_manager;
    };

    class SvoClient{
    public:
        SvoClient() = default;
        ~SvoClient() = default;

        void  readUnblocking(RealtimeObjectBase& item,
                             FcnCallbackInterface* callback=nullptr);

        void writeUnblocking(RealtimeObjectBase& item,
                             FcnCallbackInterface* callback=nullptr);

#ifdef SYSTYPE_FULL_OS
//        void  readBlocking(RealtimeObjectBase& item);
//        void writeBlocking(RealtimeObjectBase& item);
#endif
        uint16_t server_addr { 0 };

    private:
        RealtimeObjectDict* const svo_dict{nullptr};
    };


}

#endif //LIBFCN_V2_SERVICEOBJECT_HPP
