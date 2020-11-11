//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_PARAMSERVER_HPP
#define LIBFCN_V2_PARAMSERVER_HPP

#include <cstdint>
#include <memory>

#include "utils/BitLUT8.hpp"
#include "utils/Tracer.hpp"
#include "utils/vector_s.hpp"
#include "utils/CppUtils.hpp"

#include "DataLinkLayer.hpp"
#include "SerDesDict.hpp"
#include "DefaultAllocate.h"
#include "OpCode.hpp"
#include "DefaultAllocate.h"



#ifdef USE_REQUEST_EVLOOP
#include "ParamServerRequestEv.hpp"
#endif

uint64_t globalTimeSourceMS();

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

    class ParamServerManager;
    class NetworkLayer;

    class ParamServerClient{
    public:
        //TODO: 传入SerDesDict，在回调中实现类型安全的反序列化
        ParamServerClient(NetworkLayer* ctx_network_layer,
                          uint16_t server_addr,
                          uint16_t client_addr,
                          uint16_t port_id) :

                server_addr(server_addr),
                client_addr(client_addr),
                ctx_network_layer(ctx_network_layer),
                port_id(port_id)              //TODO: noport, broadcast!


#ifdef USE_REQUEST_EVLOOP

    #ifdef SYSTYPE_FULL_OS
                    ,ev_loop(utils::evloopTimeSrouceMS, 0)
    #else //SYSTYPE_FULL_OS
    				,ev_loop(globalTimeSourceMS, CLIENT_MAX_REQ_NUM)
    #endif //SYSTYPE_FULL_OS

#endif //USE_REQUEST_EVLOOP
                  {}

        ~ParamServerClient() = default;

        template<typename Prototype>
        static Prototype readBuffer(Prototype&& msg, FcnFrame* frame){
            USER_ASSERT(frame!= nullptr);
            USER_ASSERT(frame->msg_id == msg.index);
            USER_ASSERT(frame->getPayloadLen() == sizeof(msg.data));

            Prototype res = msg;
            utils::memcpy(&res.data, frame->payload, sizeof(res.data));

            return res;
        }

        template<typename Msg>
        void readUnblocking(Msg&& msg,
                            RequestCallback&& callback=RequestCallback(),
                            uint16_t timeout_ms=300, int retry=3){
            //TODO: local first

            FcnFrame frame;
            frame.dest_id = server_addr;
            frame.src_id  = client_addr;
            frame.op_code = (uint8_t)OpCode::ParamServer_ReadReq;
            frame.msg_id = msg.index;
            frame.setPayloadLen(1);
            frame.payload[0] =  msg.data_size;

#ifndef USE_REQUEST_EVLOOP
            networkSendFrame(port_id, &frame);
#else //USE_REQUEST_EVLOOP
            int res = ev_loop.addTask(std::make_unique<ParamServerRequestEv>(
                    this,
                    frame,
                    (uint8_t)OpCode::ParamServer_ReadAck,
                    timeout_ms, retry,
                    std::move(callback))
                );
            if(res == -1){
                LOGW("evloop can't add more task!");
            }
#endif //USE_REQUEST_EVLOOP
        }


        template<typename Msg>
        void writeUnblocking(Msg&& msg,
                             RequestCallback&& callback=RequestCallback(),
                             uint16_t timeout_ms=300, int retry=3){
            //TODO: local first

            FcnFrame frame;
            frame.dest_id = server_addr;
            frame.src_id  = client_addr;
            frame.op_code = (uint8_t)OpCode::ParamServer_WriteReq;
            frame.msg_id = msg.index;
            frame.setPayloadLen(msg.data_size);
            utils::memcpy(frame.payload, &msg.data, msg.data_size);

#ifndef USE_REQUEST_EVLOOP
            networkSendFrame(port_id, &frame);
#else //USE_REQUEST_EVLOOP
            int res = ev_loop.addTask(std::make_unique<ParamServerRequestEv>(
                    this,
                    frame,
                    (uint8_t)OpCode::ParamServer_WriteAck,
                    timeout_ms, retry,
                    std::move(callback))
            );
            if(res == -1){
                LOGW("evloop can't add more task!");
            }
#endif //USE_REQUEST_EVLOOP
        }


#ifdef SYSTYPE_FULL_OS
//        template<typename Msg>
//        typename Msg::data readBlocking(Msg&& item){}
//
//
//
//        template<typename Msg>
//        typename Msg::data writeBlocking(Msg&& item,
//                             FcnCallbackInterface* callback=nullptr){
//
//        }
#endif
        //TODO: const
        uint16_t server_addr { 0 };
        uint16_t client_addr { 0 };
        uint16_t port_id{0};

    private:
        friend class ParamServerManager;
        friend class ParamServerRequestEv;

        SerDesDict* const serdes_dict{nullptr};

        int networkSendFrame(uint16_t port_id, FcnFrame* frame);

        NetworkLayer* const ctx_network_layer{nullptr};

        void onReadAck(FcnFrame* frame);

        void onWriteAck(FcnFrame* frame);

#ifdef USE_REQUEST_EVLOOP
       FcnEvLoop ev_loop;
#endif
    };


    /*
     * 以回调响应的，称为RPC模式的SVO。
     * RPC无数据存储实例，仅有一个回调。且不同RPC回调实例可以处理不同格式的请求数据帧。
     *
     * 读写数据的，称为寄存器模式的SVO。
     * 寄存器模式下，可以有回调，也可以没有。一般写入会有回调，读取没有。
     * */
    class ParamServer{
    public:
        ParamServer(NetworkLayer* ctx_network_layer,
                    uint16_t address, SerDesDict* obj_dict_shm, void* buffer):
                server_addr(address),
                buffer(buffer),
                serdes_dict(obj_dict_shm),
                ctx_network_layer(ctx_network_layer){
        }

        ~ParamServer() = default;

        //TODO: 任何表项目被从网络写入，均回调
        void onDataChaged(SerDesDictValHandle* msg,
                          TransferCallback_t* callback);


        //TODO: 对应数据的网络写入回调
        template<typename Msg>
        void onDataChanged(Msg&& item, TransferCallback_t* callback){

        }

        template<typename Msg>
        void updateData(Msg&& msg){
            serdes_dict->serialize(msg, buffer);
        }

        template<typename Prototype>
        Prototype getData(Prototype&& msg){
            USER_ASSERT(buffer!= nullptr);
            return serdes_dict->deserialize(msg, buffer);
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

    private:
        friend class ParamServerManager;


        uint16_t server_addr { 0 };

        utils::BitLUT8 wr_access_table;

        void* buffer{nullptr};

        SerDesDict* const serdes_dict{nullptr};


        /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
        obj_size_t onWriteReq(FcnFrame* frame, uint16_t port_id);

        obj_size_t onReadReq(FcnFrame* frame, uint16_t port_id);

        NetworkLayer* const ctx_network_layer{nullptr};
    };


    /*
    * 网络处理。
    * 不论本地有几个节点，节点均共享一个该实例（单例模式）
    * 但为了降低耦合度，这里不实现单例模式，由上层实现。
    * */
    class ParamServerManager{
    public:
        ParamServerManager(NetworkLayer* ctx_network_layer):
                ctx_network_layer(ctx_network_layer),
                created_servers(MAX_LOCAL_NODE_NUM),
                created_clients(MAX_LOCAL_NODE_NUM)
        {}

        ~ParamServerManager() = default;

        /* 不同于Pub-Sub，一个地址只允许存在一个服务器实例 */
        ParamServer* createServer(SerDesDict& prototype, uint16_t address);

        ParamServerClient* bindClientToServer(uint16_t server_addr,
                                              uint16_t client_addr,
                                              uint16_t port_id);

        int handleRecv(FcnFrame* frame, uint16_t recv_port_id);

    private:

        NetworkLayer* const ctx_network_layer{nullptr};

        struct CreatedServer{
            int         address {-1};
            ParamServer*  instance {nullptr};
        };

        utils::vector_s<CreatedServer> created_servers;


        struct CreatedClient{
            int         address {-1};
            ParamServerClient*  instance {nullptr};
        };

        utils::vector_s<CreatedClient> created_clients;

    };


}

#endif //LIBFCN_V2_PARAMSERVER_HPP
