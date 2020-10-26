//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_SERVICEOBJECT_HPP
#define LIBFCN_V2_SERVICEOBJECT_HPP

#include <cstdint>
#include "utils/vector_s.hpp"
#include "DataLinkLayer.hpp"
#include "DataObjects.hpp"

namespace libfcn_v2 {

    class NetworkLayer;


    /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
    obj_size_t svoServerWrite(ServiceObjectDict* dict, obj_idx_t index, uint8_t
    *data, obj_size_t len);

    void svoReadAckHandle(ServiceObjectDict* dict, obj_idx_t index, uint8_t *data, obj_size_t len);

    void svoWriteAckHandle(ServiceObjectDict* dict, obj_idx_t index, uint8_t result);


    class SvoServer{
    public:
        SvoServer();
        ~SvoServer() = default;
//        void handleRecv(DataLinkFrame* frame, uint16_t recv_port_id);

        ServiceObjectDict* dict{nullptr};

        NetworkLayer* network{nullptr};

        uint16_t address{0};

        uint8_t is_server{0};

        void handleRecv(DataLinkFrame *frame, uint16_t recv_port_id);
    };

    class SvoClient{
    public:
        SvoClient();
        ~SvoClient();

        void  readUnblocking(RealtimeObjectBase& item, FcnCallbackInterface callback);
        void writeUnblocking(RealtimeObjectBase& item, FcnCallbackInterface callback);

#ifdef SYSTYPE_FULL_OS
        void  readBlocking(RealtimeObjectBase& item);
        void writeBlocking(RealtimeObjectBase& item);
#endif
        uint16_t server_addr { 0 };

    private:
        RealtimeObjectDict* svo_dict{nullptr};
    };


}

#endif //LIBFCN_V2_SERVICEOBJECT_HPP
