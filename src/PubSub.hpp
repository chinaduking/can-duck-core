//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_PUBSUB_HPP
#define LIBFCN_V2_PUBSUB_HPP

#include <cstdint>
#include "Vector.hpp"
#include "LinkedList.hpp"
#include "DataLinkLayer.hpp"
#include "ObjDict.hpp"
#include "OpCode.hpp"
#include "DefaultAllocate.h"

namespace libfcn_v2 {
    /* ---------------------------------------------------------
     * 前置声明
     * ---------------------------------------------------------*/
    struct SubscribeCallback;

    class NetworkLayer;
    class PubSubManager;
    class Publisher;
    class Subscriber;

    /* --------------------------------------------------------- */
    /**
     * @brief 构造一个数据帧
     *
     * @author  sdong
     * @date    2020/10/15
     */
    void singleWriteFrameBuilder(
            FcnFrame* result_frame,
            uint16_t src_id,
            uint16_t dest_id,
            uint16_t op_code,
            uint16_t msg_id,
            uint8_t* p_data, uint16_t len);


    /* --------------------------------------------------------- */

    /**
     * @brief 将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度
     *
     * @author  sdong
     * @date    2020/10/15
     */
    obj_size_t RtoDictSingleWrite(ObjDict* dict,
                                  void* buffer,
                                  obj_idx_t index,
                                  uint8_t *data, obj_size_t len);

    obj_size_t RtoBufferWrite(void* buffer,
                              obj_idx_t index,
                              uint8_t *data, obj_size_t len);

    /* --------------------------------------------------------- */

    #define MAX_PUB_CTRL_RULES 10  //TODO: LINKED LIST

    /**
     * @brief 发布订阅管理器，管理所有通道实例，进行本地和网络转发处理
     *
     * @details 不论本地有几个节点，节点均共享一个该实例（单例模式）
     *      但为了降低耦合度，这里不实现单例模式，由上层实现。
     * @author  sdong
     * @date    2020/10/15
     */
    class PubSubManager{
    public:
        /* ---------- Constructors ---------  */
        explicit PubSubManager(NetworkLayer* network)
            : network_layer(network)
        { }


        /* ----------- Destructor ----------  */
        ~PubSubManager() = default;


        std::pair<Publisher*, Subscriber*> bindPubChannel(ObjDict& serdes_dict_tx,
                                                          ObjDict& serdes_dict_rx,
                                                          uint16_t node_id,
                                                          bool is_owner_node);

        /* --------- Public Methods --------  */
//        Publisher* makePublisher(ObjDict& serdes_dict,
//                                 uint16_t node_id,
//                                 bool is_owner, bool is_fast_msg=true);
//
//        Subscriber* makeSubscriber(ObjDict& serdes_dict,
//                                   uint16_t node_id,
//                                   bool is_owner, bool is_fast_msg=true);

        void handleWrtie(FcnFrame* frame, uint16_t recv_port_id);

        /* ------- Public Variables --------  */
        NetworkLayer* const network_layer{nullptr};

    protected:
        /* ------ Protected Declarations ------  */
        struct SharedBuffer{
            int32_t    id {-1};
            void*  buffer {nullptr};
        };


        /* --------- Protected Methods --------  */

        /* ------- Protected Variables --------  */


        uint16_t poll_freq_hz{1000};

        emlib::LinkedList<SharedBuffer> shared_buffers;

        emlib::LinkedList<Publisher *> created_publishers;

        emlib::LinkedList<Subscriber*> created_subscribers;


        Publisher* bindPublisherToChannel(ObjDict& serdes_dict, uint16_t node_id, bool is_owner);
        Subscriber* bindSubscriberToChannel(ObjDict& serdes_dict, uint16_t node_id, bool is_owner);


        void* getSharedBuffer(ObjDict& serdes_dict, uint16_t id, uint8_t sub_id);
    };

    /* --------------------------------------------------------- */

    /**
     * @berif 发布者
     *
     * @details 主要持有：
     *  本地共享的订阅者列表指针
     */
    class Publisher{
    public:
        /* ------ Public Declarations ------  */
        /* ---------- Constructors ---------  */
        /**
         * @brief 构造函数
         *
         * @details dstid为通道id，通道id等于通道中源节点id。源节点id小于预留组播id。
         * 如果数据包dstid=src，证明是源节点数出数据包(mosi)：此时，如果从节点配置了相应id
         * 的通道，则接收数据（按目标id分发（注意不是按源id分发，每个主节点有自己的通道！））
         * 否则，如果dstid≠srcid，但dstid=nodeid，证明是从节点输出的数据包miso。此时主节
         * 点将数据填入自己的状态表，无需分发。如果dstid不等于srcid且dstid也属于组播ID
         * (因此不等于任何一个nodeid），则配置了相应组播id的节点进行处理。
         *
         * @param node_id 通道ID，为数据帧中的目标ID。当为单主通道添加主发布者时，
         *                   node_id = src_id。当为多主通道添加从发布者时，通道ID为对应
         *                   主节点ID。当为多主通道添加发布者时，channel_id属于组播ID之一。
         *
         * @param src_id 源ID，为数据帧中的源ID。总是等于用户为节点配置的ID。ID必须是全
         *               网唯一的，且不可处于组播ID的范围内。
         *
         * @param ps_manager PubSubManager指针。管理所有已创建的发布者、订阅者和共享内存。
         *                  不能为空。
         */
        Publisher(ObjDict& serdes_dict,
                  void* buffer,
                  uint16_t node_id,
//                   uint16_t src_id,
                   PubSubManager* ps_manager) :

                node_id(node_id),
//                src_id(src_id),

                serdes_dict(&serdes_dict),
                buffer(buffer),
                ps_manager(ps_manager)
        { }

        /* ----------- Destructor ----------  */
        ~Publisher() = default;

        /* --------- Public Methods --------  */

        /**
         * @brief 向通道中发布一个消息
         *
         * @param msg
         */
        void publish(ObjDictValHandle& msg, bool local_only=false);

        /**
         * @brief 注册本地发布者
         *
         * @param subscriber
         */
        void regLocalSubscriber(Subscriber* subscriber);

        Publisher& addPort(int port);

        /* ------- Public Variables --------  */
        const uint16_t node_id {0 };
//        const uint16_t src_id     { 0 };
        bool is_owner {false};

    private:
        /* ------ Private Declarations ------  */
        /* --------- Private Methods --------  */

        /* ------- Private Variables --------  */
        ObjDict*    const serdes_dict {nullptr };
        void*          const buffer      { nullptr };
        PubSubManager* const ps_manager  { nullptr };

        FcnFrame trans_frame_tmp;

        emlib::LinkedList<Subscriber*> local_sub_ptr;
        emlib::LinkedList<int> network_pub_ports;

    };

    /* --------------------------------------------------------- */
    /**
     * @brief 订阅实时消息的回调
     *
     * @author  sdong
     * @date    2020/11/13
     */
    struct SubscribeCallback{
        /* ------ Public Declarations ------  */
        typedef void (*Callback)(void* p_this, Subscriber* channel);

        /* ---------- Constructors ---------  */
        SubscribeCallback() = default;

        SubscribeCallback(Callback cb, void* p_this=nullptr):
                cb(cb), p_this(p_this)
        {}

        /* --------- Public Methods --------  */
        inline void call(Subscriber* subscriber){
            if(cb != nullptr){
                (*cb)(p_this, subscriber);
            }
        }

        /* ------- Public Variables --------  */
        Callback cb {nullptr};
        void* p_this {nullptr};
    };

    #define FCN_SUBSCRIBE_CALLBACK(fname) void fname(void* p_this, \
                    Subscriber* subscriber)

    /* --------------------------------------------------------- */

    /**
     * @berif 订阅者
     *
     * @details 主要持有：
     *  数据缓冲区及反序列化字典：单主主节点输入数据；单主从节点输入数据（主节点上为输出）；多主输入输出数据
     *  回调列表
     *  指令计数表
     *
     */
    class Subscriber{
    public:
        Subscriber(ObjDict& serdes_dict,
                   void* buffer,
                   uint16_t channel_id,
//                    uint16_t src_id,
                    PubSubManager* ps_manager) :

                node_id(channel_id),
//                src_id(src_id),

                serdes_dict(&serdes_dict),
                buffer(buffer),
                ps_manager(ps_manager),
                callback_table(serdes_dict.dictSize()) {
                    callback_table.resize(serdes_dict.dictSize());
                }

        /* ----------- Destructor ----------  */
        ~Subscriber() = default;

        /* --------- Public Methods --------  */
        void subscribe(ObjDictValHandle& handle,
                       SubscribeCallback::Callback cb_func,
                       void* p_this = nullptr){
            callback_table[handle.index].cb = cb_func;
            callback_table[handle.index].p_this = p_this;
        }

        template<typename Prototype>
        inline Prototype readBuffer(Prototype&& msg){
            USER_ASSERT(buffer!= nullptr);
            return serdes_dict->deserialize(msg, buffer);
        }

        void notify(uint16_t index){
            callback_table[index].call(this);
        }

        /* ------- Public Variables --------  */
        const uint16_t node_id {0 };
//        const uint16_t src_id     { 0 };
        bool is_owner {false};

    private:
        /* ------ Private Declarations ------  */
        friend class PubSubManager;
        /* --------- Private Methods --------  */

        /* ------- Private Variables --------  */
        ObjDict*    const serdes_dict {nullptr };
        void*          const buffer      { nullptr };
        PubSubManager* const ps_manager  { nullptr };

        emlib::Vector<SubscribeCallback> callback_table;

    };

}



#endif //LIBFCN_V2_PUBSUB_HPP
