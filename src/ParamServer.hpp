//
// Created by sdong on 2020/10/15.
//

#ifndef can_duck_PARAMSERVER_HPP
#define can_duck_PARAMSERVER_HPP

#include <cstdint>
#include <memory>

#include "BitLUT8.hpp"
#include "Tracer.hpp"
#include "Vector.hpp"
#include "CppUtils.hpp"

#include "SerDesDict.hpp"
#include "DefaultAllocate.h"
#include "OpCode.hpp"
#include "DefaultAllocate.h"
#include "Common.hpp"

#ifdef USE_REQUEST_EVLOOP
#include "ClientRequestEv.hpp"
#endif

uint64_t globalTimeSourceMS();

namespace can_duck {

    class ParamServerManager;
//    class NetworkLayer;

    class ParamServerClient{
    public:
        ParamServerClient(ParamServerManager* manager,
                          uint16_t server_addr,
                          uint16_t client_addr,
                          uint16_t port_id,
                          SerDesDict* serdes_dict,
                          void* buffer) :

                server_addr(server_addr),
                client_addr(client_addr),
                manager(manager),
                port_id(port_id)              //TODO: noport, broadcast!
                ,serdes_dict(serdes_dict)
                ,buffer(buffer)

#ifdef USE_REQUEST_EVLOOP

    #ifdef SYSTYPE_FULL_OS
                    ,ev_loop(emlib::evloopTimeSrouceMS, 0)
    #else //SYSTYPE_FULL_OS
    				,ev_loop(globalTimeSourceMS, CLIENT_MAX_REQ_NUM)
    #endif //SYSTYPE_FULL_OS

#endif //USE_REQUEST_EVLOOP
                  {}

        ~ParamServerClient() = default;

        template<typename Prototype>
        Prototype readBuffer(Prototype&& msg){
            USER_ASSERT(buffer!= nullptr);
            return serdes_dict->deserialize(msg, buffer);
        }

        template<typename Msg>
        void readAsync(Msg&& msg,
                       RequestCallback::Callback callback_func = nullptr,
                       void* callback_ctx_obj = nullptr,
                       uint16_t timeout_ms= 300, int retry= 3){
            //TODO: local first

            ServiceFrame frame;
            frame.dest_id = server_addr;
            frame.src_id  = client_addr;
            frame.op_code = (uint8_t)OpCode::ParamServer_ReadReq;
            frame.srv_id  = msg.index;
            frame.payload_len = 1;
            frame.payload[0] =  msg.data_size;

            #ifndef USE_REQUEST_EVLOOP
            networkSendFrame(port_id, &frame);
#else //USE_REQUEST_EVLOOP
            int res = ev_loop.addTask(
                    std::make_unique<ClientRequestEv>(
                        this,
                        frame,
                        (uint8_t)OpCode::ParamServer_ReadAck,
                        timeout_ms, retry,
                        RequestCallback(callback_func, callback_ctx_obj)
                    )
                );

            if(res == -1){
                LOGW("evloop can't add more task!");
            }
#endif //USE_REQUEST_EVLOOP
        }


        template<typename Msg>
        void writeAsync(Msg&& msg,
                        RequestCallback::Callback callback_func = nullptr,
                        void* callback_ctx_obj = nullptr,
                        uint16_t timeout_ms= 300, int retry= 3){
            //TODO: local first

            ServiceFrame frame;
            frame.dest_id = server_addr;
            frame.src_id  = client_addr;
            frame.op_code = (uint8_t)OpCode::ParamServer_WriteReq;
            frame.srv_id = msg.index;
            frame.payload_len = msg.data_size;
            emlib::memcpy(frame.payload, &msg.data, msg.data_size);

#ifndef USE_REQUEST_EVLOOP
            networkSendFrame(port_id, &frame);
#else //USE_REQUEST_EVLOOP
            int res = ev_loop.addTask(
                    std::make_unique<ClientRequestEv>(
                        this,
                        frame,
                        (uint8_t)OpCode::ParamServer_WriteAck,
                        timeout_ms, retry,
                        RequestCallback(callback_func, callback_ctx_obj)
                    )
                );
            if(res == -1){
                LOGW("evloop can't add more task!");
            }
#endif //USE_REQUEST_EVLOOP
        }


#ifdef SYSTYPE_FULL_OS
//        template<typename Msg>
//        typename Msg::data readSync(Msg&& item){}
//
//
//
//        template<typename Msg>
//        typename Msg::data writeSync(Msg&& item,
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
        friend class ClientRequestEv;

        SerDesDict* const serdes_dict{nullptr};

        int networkSendFrame(uint16_t port_id, ServiceFrame* frame);

//        NetworkLayer* const ctx_network_layer{nullptr};
        ParamServerManager* const manager;


        void onReadAck(ServiceFrame* frame);

        void onWriteAck(ServiceFrame* frame);

        bool updateData(uint16_t index, uint8_t* data){
            return serdes_dict->deserialize(index, data, buffer);
        }


#ifdef USE_REQUEST_EVLOOP
       FcnEvLoop ev_loop;
#endif
        void* const buffer{nullptr};
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
        ParamServer(ParamServerManager* manager,
                    uint16_t address, SerDesDict* obj_dict_shm, void* buffer):
                manager(manager),
                server_addr(address),
                buffer(buffer),
                serdes_dict(obj_dict_shm)
                {
        }

        ~ParamServer() = default;

        //TODO: 任何表项目被从网络写入，均回调
        void onDataChaged(hDictItem* msg,
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
        /* ------ Private Declaration ------  */
        friend class ParamServerManager;

        /* ------ Private Methods ------  */
        obj_size_t onWriteReq(ServiceFrame* frame, uint16_t port_id);

        obj_size_t onReadReq(ServiceFrame* frame, uint16_t port_id);


        /* ------ Private Data ------  */
        uint16_t server_addr { 0 };

        void* const buffer{nullptr};

        emlib::BitLUT8 wr_access_table;

        SerDesDict* const serdes_dict{nullptr};

//        NetworkLayer* const ctx_network_layer{nullptr};

        ParamServerManager* const manager;
    };


    /*
    * 网络处理。
    * 不论本地有几个节点，节点均共享一个该实例（单例模式）
    * 但为了降低耦合度，这里不实现单例模式，由上层实现。
    * */
    class ParamServerManager{
    public:
        ParamServerManager(LLCanBus* can):
                can(can),
                created_servers(MAX_LOCAL_NODE_NUM),
                created_clients(MAX_LOCAL_NODE_NUM)
        {}

        ~ParamServerManager() = default;

        /* 不同于Pub-Sub，一个地址只允许存在一个服务器实例 */
        ParamServer* createServer(SerDesDict& prototype, uint16_t address);

        ParamServerClient* bindClientToServer(
                SerDesDict& prototype,
                uint16_t server_addr,
                  uint16_t client_addr,
                  uint16_t port_id);

        int handleRecv(CANMessage* can_msg, uint16_t recv_port_id);
        int handleRecv(ServiceFrame* srv_frame, uint16_t recv_port_id);

        inline int sendFrame(CANMessage& msg){
            if(can == nullptr){
                return 0;
            }
            return  can->write(msg);
        }

    private:

        LLCanBus* const can{nullptr};
//        NetworkLayer* const ctx_network_layer{nullptr};


        struct CreatedServer{
            int         address {-1};
            ParamServer*  instance {nullptr};
        };

        emlib::Vector<CreatedServer> created_servers;


        struct CreatedClient{
            int         address {-1};
            ParamServerClient*  instance {nullptr};
        };

        emlib::Vector<CreatedClient> created_clients;

    };


}

#endif //can_duck_PARAMSERVER_HPP
