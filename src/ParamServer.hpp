//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_PARAMSERVER_HPP
#define LIBFCN_V2_PARAMSERVER_HPP

#include <cstdint>
#include "utils/vector_s.hpp"
#include "DataLinkLayer.hpp"
#include "SerDesDict.hpp"
#include "DefaultAllocate.h"
#include "OpCode.hpp"
#include "utils/BitLUT8.hpp"

namespace libfcn_v2 {
    /*参数表任务状态：包括读、写*/
    enum class SvoClientStat : uint8_t {
        Idle = 0,   /*初始状态/无任务状态*/
        Pendding,   /*正在进行读写*/
        Ok,         /*读写成功*/
        Rejected,   /*访问被拒绝（可能原因：服务器上的元信息和数据包不匹配、没有权限）*/
        Timeout,    /*访问超时（可能原因：网络层通信失败）*/
        Unknown     /*未知错误*/
    };

    class SvoNetworkHandler;
    class NetworkLayer;


    class SvoClient{
    public:
        SvoClient(NetworkLayer* network_layer,
                  uint16_t server_addr,
                  uint16_t client_addr,
                  uint16_t port_id) :

                    server_addr(server_addr),
                    client_addr(client_addr),
                    network_layer(network_layer),
                    pending_reqs(CLIENT_MAX_REQ_NUM),
                    port_id(port_id)
                  {}

        ~SvoClient() = default;

        template<typename Msg>
        void readUnblocking(Msg&& msg,
                            FcnCallbackInterface* callback=nullptr){
            DataLinkFrame frame;
            frame.dest_id = server_addr;
            frame.src_id  = client_addr;
            frame.op_code = (uint8_t)OpCode::SVO_SINGLE_READ_REQ;
            frame.msg_id = msg.index;
            frame.payload_len = 1;
            frame.payload[0] =  msg.data_size;

            /* TODO: 将数据和回调指针推入任务列表（事件循环），等待响应回调 */
            networkSendFrame(port_id, &frame);
        }


        template<typename Msg>
        void writeUnblocking(Msg&& msg,
                             FcnCallbackInterface* callback=nullptr){
            DataLinkFrame frame;
            frame.dest_id = server_addr;
            frame.src_id  = client_addr;
            frame.op_code = (uint8_t)OpCode::SVO_SINGLE_WRITE_REQ;
            frame.msg_id = msg.index;
            frame.payload_len = msg.data_size;
            utils::memcpy(frame.payload, &msg.data, msg.data_size);

            /* TODO: 将数据和回调指针推入任务列表（事件循环），等待响应回调 */
            networkSendFrame(port_id, &frame);
        }

#ifdef SYSTYPE_FULL_OS
//        template<typename Msg>
//        typename Msg::data readUnblocking(Msg&& item){}
//
//
//        /* TODO: 将数据和回调指针推入任务列表（事件循环），等待响应回调 */
//        template<typename Msg>
//        typename Msg::data writeUnblocking(Msg&& item,
//                             FcnCallbackInterface* callback=nullptr){
//
//        }
#endif
        //TODO: const
        uint16_t server_addr { 0 };
        uint16_t client_addr { 0 };

    private:
        int networkSendFrame(uint16_t port_id, DataLinkFrame* frame);


        NetworkLayer* const network_layer{nullptr};

        friend class SvoNetworkHandler;

        void onReadAck(DataLinkFrame* frame);

        void onWriteAck(DataLinkFrame* frame);

        struct PendingRequest{
            uint16_t server_address;
            uint32_t timeout_time_100ms;
            FcnCallbackInterface* callback;
        };
        utils::vector_s<PendingRequest> pending_reqs;

        uint16_t port_id{0};
    };

    /*
     * 以回调响应的，称为RPC模式的SVO。
     * RPC无数据存储实例，仅有一个回调。且不同RPC回调实例可以处理不同格式的请求数据帧。
     *
     * 读写数据的，称为寄存器模式的SVO。
     * 寄存器模式下，可以有回调，也可以没有。一般写入会有回调，读取没有。
     * */
    class SvoServer{
    public:
        SvoServer(NetworkLayer* network_layer,
                  uint16_t address, SerDesDict* obj_dict_shm, void* buffer):
                server_addr(address),
                buffer(buffer),
                obj_dict_prototype(obj_dict_shm),
                network_layer(network_layer){
        }

        ~SvoServer() = default;

        //TODO: 任何表项目被从网络写入，均回调
        void onDataChaged(SerDesPrototypeHandle* msg,
                          FcnCallbackInterface* callback);


        //TODO: 对应数据的网络写入回调
        template<typename Msg>
        void onDataChanged(Msg&& item, FcnCallbackInterface* callback){

        }

        utils::BitLUT8 wr_access_table;

        uint16_t server_addr { 0 };

        template<typename Msg>
        void updateData(Msg&& msg){
            obj_dict_prototype->write(msg, buffer);
        }

        template<typename Prototype>
        Prototype getData(Prototype&& msg){
            USER_ASSERT(buffer!= nullptr);
            return obj_dict_prototype->read(msg, buffer);
        }

        template<typename Prototype>
        void setWrAccess(Prototype&& msg){
            wr_access_table.add(msg.index);
        }

        template<typename Prototype>
        void resetWrAccess(Prototype&& msg){
            wr_access_table.remove(msg.index);
        }


        /*
         * TODO:
         * 回调分配在堆上。堆为连续的，插入时会将不够的空间向后推。
         * 该方法插入很慢，查找很快，空间利用率很高。
         * 注意处理堆变更时指针更新问题。
         *
         * ObjDict所有成员均为静态成员，数据作为消息的原型（Prototype）
         * 因此不包含回调、状态码等运行时可改变的信息。
         *
         * 运行时存储的回调，为回调堆上的地址偏移。
         * */
//        uint16_t callback_offset[dict_size];


        void* buffer{nullptr};

//    private:
        SerDesDict* const obj_dict_prototype{nullptr};


        friend class SvoNetworkHandler;

        /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
        obj_size_t onWriteReq(DataLinkFrame* frame, uint16_t port_id);

        obj_size_t onReadReq(DataLinkFrame* frame, uint16_t port_id);

        NetworkLayer* const network_layer{nullptr};
    };


    /*
    * 网络处理。
    * 不论本地有几个节点，节点均共享一个该实例（单例模式）
    * 但为了降低耦合度，这里不实现单例模式，由上层实现。
    * */
    class SvoNetworkHandler{
    public:
        SvoNetworkHandler(NetworkLayer* network):
                network(network),
                created_servers(MAX_LOCAL_NODE_NUM),
                created_clients(MAX_LOCAL_NODE_NUM)
        {}


        virtual ~SvoNetworkHandler() = default;

        /* 不同于Pub-Sub，一个地址只允许存在一个服务器实例 */
        SvoServer* createServer(SerDesDict& prototype, uint16_t address){
            SvoServer* server = nullptr;
            for(auto & srv : created_servers){
                if(srv.address == address){
                    server = srv.instance;
                    USER_ASSERT(server != nullptr);
                }
            }
            if(server == nullptr){
                server = new SvoServer(network, address,
                                       &prototype,
                                       prototype.createBuffer());

                CreatedServer srv = {
                        .address = address,
                        .instance = server
                };

                created_servers.push_back(srv);
            }

            return server;
        }

        SvoClient* bindClientToServer(uint16_t server_addr,
                                      uint16_t client_addr,
                                      uint16_t port_id){
            auto client = new SvoClient(network, server_addr, client_addr,
                                        port_id);

            CreatedClient cli = {
                    .address = client_addr,
                    .instance = client
            };

            created_clients.push_back(cli);

            return client;
        }

        int handleRecv(DataLinkFrame* frame, uint16_t recv_port_id);

        NetworkLayer* network{nullptr};

        uint8_t is_server{0};


        struct CreatedServer{
            int         address {-1};
            SvoServer*  instance {nullptr};
        };

        utils::vector_s<CreatedServer> created_servers;


        struct CreatedClient{
            int         address {-1};
            SvoClient*  instance {nullptr};
        };

        utils::vector_s<CreatedClient> created_clients;

    };


}

#endif //LIBFCN_V2_PARAMSERVER_HPP
