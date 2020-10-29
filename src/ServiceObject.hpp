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


    class SvoClient{
    public:
        SvoClient() = default;
        ~SvoClient() = default;

        void readUnblocking(ServiceObjectBase& item,
                            FcnCallbackInterface* callback=nullptr);

        void writeUnblocking(ServiceObjectBase& item,
                             FcnCallbackInterface* callback=nullptr);

#ifdef SYSTYPE_FULL_OS
//        void  readBlocking(ServiceObjectBase& item);
//        void writeBlocking(ServiceObjectBase& item);
#endif
        uint16_t server_addr { 0 };

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
//        ServiceObjectDict* const obj_dict{nullptr};
    };

    class SvoServer{
    public:
        SvoServer() = default;
        ~SvoServer() = default;

        void readUnblocking(ServiceObjectBase& item,
                            FcnCallbackInterface* callback=nullptr);

#ifdef SYSTYPE_FULL_OS
//        void  readBlocking(ServiceObjectBase& item);
//        void writeBlocking(ServiceObjectBase& item);
#endif
        uint16_t server_addr { 0 };

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

        /*
         * 服务器端除了可以用回调响应请求，还可以直接通过数据源响应请求
         * 偏移表存储每个数据源的偏移（相当于一个小指针的数组）
         * -通过偏移表成员本身的地址偏移可以求出index
         * -通过偏移表前后成员做差可以求出size
         *
         * 以回调响应的，称为RPC模式的SVO。
         * RPC无数据存储实例，仅有一个回调。且不同RPC回调实例可以处理不同格式的请求数据帧。
         *
         * 读写数据的，称为寄存器模式的SVO。
         * 寄存器模式下，可以有回调，也可以没有。一般写入会有回调，读取没有。
         * */
        void* data_src{nullptr};

        private:
            ServiceObjectDict* const obj_dict{nullptr};
        };





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

        template<typename T_Dict>
        ServiceObjectDict* bindDictAsServer(uint16_t address){
            dict_manager.create<T_Dict>(address);
        }


        template<typename T_Dict>
        SvoClient* bindDictAsClient(uint16_t address){
            dict_manager.create<T_Dict>(address);

            auto client = new SvoClient();
            client->server_addr = address;

            return client;
        }

        void handleRecv(DataLinkFrame* frame, uint16_t recv_port_id);

        NetworkLayer* network{nullptr};

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


}

#endif //LIBFCN_V2_SERVICEOBJECT_HPP
