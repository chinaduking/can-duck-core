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

    /* --------------------------------------------------------- */

    /**
     *
     * @brief 发布-订阅消息通道实例
     *
     * @details 一个每一个消息频道应被分配一个全网唯一地址；
     * 且同一通道内的消息对应唯一的字典进行消息解析。
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
        PubSubChannel(SerDesDict* serdes_dict, void* buffer,
                      bool is_source= true)
            :
            serdes_dict(serdes_dict),
            buffer(buffer),
            callback_ptr_list(serdes_dict->dictSize())
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
            uint16_t src_id = 0, dest_id = 0;

            if(!is_multi_source){
                src_id = channel_addr;
                dest_id = 0;
            } else{
                src_id = 0;
                dest_id = channel_addr;
            }

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


        /* ------- Public Variables --------  */

        SerDesDict* serdes_dict{nullptr};

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

        libfcn_v2::NetworkLayer *network_layer{nullptr};

        void networkPublish(FcnFrame* frame);


        /* 是否为 "多源通道"。TODO: const */
        bool is_multi_source{false};

        /* 通道ID TODO: const */
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
    class PublisherManager{
    public:
        /* ---------- Constructors ---------  */
        explicit PublisherManager(NetworkLayer* network)
            : ctx_network_layer(network),
              shared_buffers(MAX_LOCAL_NODE_NUM),
              pub_sub_channels(MAX_LOCAL_NODE_NUM*2),
              pub_ctrl_rules(MAX_PUB_CTRL_RULES)
              { }


        /* ----------- Destructor ----------  */
        ~PublisherManager() = default;


        /* --------- Public Methods --------  */
        PubSubChannel* createChannel(SerDesDict& prototype, uint16_t address);

        PubSubChannel* createChannel(SerDesDict& prototype, uint16_t address,
                                     void* static_buffer);

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
            friend class PublisherManager;

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
