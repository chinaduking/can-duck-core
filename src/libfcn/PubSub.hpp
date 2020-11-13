//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_PUBSUB_HPP
#define LIBFCN_V2_PUBSUB_HPP

#include <cstdint>
#include "utils/Vector.hpp"
#include "DataLinkLayer.hpp"
#include "SerDesDict.hpp"
#include "OpCode.hpp"
#include "DefaultAllocate.h"

namespace libfcn_v2 {
    /* ---------------------------------------------------------
     * 前置声明
     * ---------------------------------------------------------*/
    class NetworkLayer;
    struct SubscribeCallback;
    class PubSubManager;
    /* --------------------------------------------------------- */
    enum class ChannelType : uint8_t {
        SingleSourceMaster = 0,
        SingleSourceSlave,
        MultiSource
    };

    /**
     * @berif 发布者
     *
     * @details 主要持有：
     *  本地共享的订阅者列表指针
     */
    class Publisher{

    };

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

    };

    /**
     *
     * @brief 发布-订阅消息通道实例
     *
     * @details 一个每一个消息频道应被分配一个全网唯一地址；
     * 且同一通道内的消息对应唯一的字典进行消息解析。
     *
     * 通道有两种模式：单主（一对多通信）和多主（多对多通信）。
     *
     * 单主模式：
     *
     *   单主有一个主节点（master node）和一个或多个从节点（slave node）。单主通道的地址等于
     * 主节点自身地址（Node ID）。单主通道上，主节点输出的数据包，源地址即为通道地址，同时采用
     * 1个bit的标志位置位，标识该数据包为源。单主通道的数据发送无需目标地址，因为任何订阅此通
     * 道的节点，均可接收到该数据包。当从节点向主节点发送数据，从节点需将目标ID指定为主节点ID，
     * （即通道ID）以便和指定主节点通信。源ID默认为0xFF，因为任何匹配ID的主节点均能接收到数据
     * 包。从节点之间不能互相同步主节点的状态表。
     *
     *   组播模式，不区分主从节点。全部节点发送数据包时，只需标明这一公用通道的ID。
     *
     * 为了目前的兼容性和安全性，ID暂定如下：
     * 单主通道主节点发送的数据包，源ID=通道ID，目标ID=0
     * 单主通道从节点发送的数据包，源ID=通道ID，目标ID=1
     * 多主通道发送的数据包，源ID=通道ID，目标ID=2
     *
     *
     * @author  sdong
     * @date    2020/10/15
     *
     */
    class PubSubChannel{
    public:

        /* ------ Public Declarations ------  */
        typedef SubscribeCallback* callbacl_ptr_t;


        /* ---------- Constructors ---------  */

        /**
         * @brief 发布-订阅消息通道实例
         *
         * @param serdes_dict 指向序列化字典的类型对象。同一地址的通道，采用同一字典类型对象。
         * @param buffer 存储数据的深度为1的缓冲区。由序列化字典类型对象创建的缓冲区。
         * @param is_source 是否自己是源
         */
        PubSubChannel(PubSubManager* ps_manager,
                      SerDesDict* serdes_dict, void* buffer,
                      ChannelType type)
            :
            ps_manager(ps_manager),
            serdes_dict(serdes_dict),
            buffer(buffer),
            callback_ptr_list(serdes_dict->dictSize()),
            type(type)
            {
            USER_ASSERT(buffer != nullptr);
        }

        /* ----------- Destructor ----------  */
        ~PubSubChannel() = default;


        /* --------- Public Methods --------  */

        /**
         * @brief 向通道中发布一个消息
         *
         * @param msg
         */
        template<typename Msg>
        void publish(Msg&& msg){
            if(type == ChannelType::SingleSourceMaster){
//                ps_manager
            }


            uint16_t src_id = 0, dest_id = 0;
            src_id = 0;
            dest_id = channel_addr;

            /* 先进行本地发布，即直接将数据拷贝到共享内存中 */
            serdes_dict->serialize(msg, buffer);

            /* TODO: 进行本地发布的回调 */

            /* 再进行网络发布：
             * TODO：根据发布管理进行分频*/
            singleWriteFrameBuilder(
                    &frame_tmp,
                    src_id,
                    dest_id,
                    static_cast<uint8_t>(OpCode::Publish),
                    msg.index,
                    (uint8_t *)msg.getDataPtr(), msg.data_size);

            networkPublish(&frame_tmp);
        }

        template<typename Prototype>
        Prototype readBuffer(Prototype&& msg){
            USER_ASSERT(buffer!= nullptr);
            return serdes_dict->deserialize(msg, buffer);
        }

        void networkPublish(FcnFrame* frame);


        /* ------- Public Variables --------  */

        SerDesDict* const serdes_dict{nullptr};

        void* const buffer {nullptr};

        utils::Vector<callbacl_ptr_t> callback_ptr_list;
#if 0
        /* 将回调函数指针映射到int8整形时，基址偏移量
         * 采用指针形式，因为存储映射表的堆会根据添加的实例数量进行调整
         * 同时会更改基址的值。用指针指向堆所分配的基址，可实现同步更新*/
        uint16_t*  callback_mapped_ptr_base;

        /* 将回调函数指针映射到int8整形后的指针数组 */
        uint8_t** callback_mapped_ptr_list;

        /* 上述数组长度 */
        uint8_t* callback_mapped_ptr_num;
#endif
        FcnFrame frame_tmp;

        PubSubManager* const ps_manager{nullptr};


        ChannelType const type;

        int channel_addr{0};
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
        typedef void (*Callback)(void* p_this, PubSubChannel* channel);

        /* ---------- Constructors ---------  */
        SubscribeCallback() = default;

        SubscribeCallback(Callback cb, void* p_this=nullptr):
                cb(cb), p_this(p_this)
        {}

        /* --------- Public Methods --------  */
        inline void call(PubSubChannel* channel){
            if(cb != nullptr){
                (*cb)(p_this, channel);
            }
        }

        /* ------- Public Variables --------  */
        Callback cb {nullptr};
        void* p_this {nullptr};
    };

    #define FCN_SUB_CALLBACK(fname) void fname(void* p_this, \
                PubSubChannel* channel)


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
    obj_size_t RtoDictSingleWrite(SerDesDict* dict,
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
            : ctx_network_layer(network),

              pub_ctrl_rules(MAX_PUB_CTRL_RULES),
              shared_buffers(MAX_LOCAL_NODE_NUM),
              pub_sub_channels(MAX_LOCAL_NODE_NUM*2)
        { }


        /* ----------- Destructor ----------  */
        ~PubSubManager() = default;


        /* --------- Public Methods --------  */
        PubSubChannel* createChannel(SerDesDict& prototype, uint16_t address,
                                     ChannelType type);

//        PubSubChannel* createChannel(SerDesDict& prototype, uint16_t address,
//                                     void* static_buffer);

        void handleWrtie(FcnFrame* frame, uint16_t recv_port_id);


        /* ------ Public Declarations ------  */
        struct PubCtrlRule{
            /* ---------- Constructors ---------  */
            PubCtrlRule() : data_link_dev(MAX_COM_PORT_NUM){}

            /* --------- Public Variables --------  */
            /* 发送频率。-1代表直接转发不过滤 */
            int16_t freq_hz     { -1 };

            /* 源地址。-1代表任意地址 */
            int16_t src_address {  -1  };

            /* 目标地址。-1代表任意地址 */
            int16_t dest_address{  -1  };

            /*
             * end_idx != -1 : start_idx
             * end_idx == -1 : single_idx
             **/
            obj_idx_t start_or_single_idx  {0};
            int16_t end_idx    { -1 };

            /* 需要转发到的端口列表 */
            utils::Vector<FrameIODevice*> data_link_dev;

        private:
            /* ------ Private Declarations ------  */
            friend class PubSubManager;

            /* ------- Private Variables --------  */
            uint32_t freq_divier{0};
            uint32_t freq_divier_cnt{0};

            uint32_t send_busy_cnt {0};

        };


        /* --------- Public Methods --------  */
        void addPubCtrlRule(PubCtrlRule& rule);

        void update();


    protected:
        /* ------ Protected Declarations ------  */
        struct SharedBuffer{
            int    id {-1};
            void*  buffer {nullptr};
        };


        /* --------- Protected Methods --------  */

        /* ------- Protected Variables --------  */

        NetworkLayer* const ctx_network_layer{nullptr};

        uint16_t poll_freq_hz{1000};

        utils::Vector<PubCtrlRule> pub_ctrl_rules;

        utils::Vector<SharedBuffer> shared_buffers;

        utils::Vector<PubSubChannel*> pub_sub_channels;

    };
}



#endif //LIBFCN_V2_PUBSUB_HPP
