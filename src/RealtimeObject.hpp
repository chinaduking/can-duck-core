//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_REALTIMEOBJECT_HPP
#define LIBFCN_V2_REALTIMEOBJECT_HPP

#include <cstdint>
#include "utils/vector_s.hpp"
#include "DataLinkLayer.hpp"
#include "DataObjects.hpp"
#include "OperationCode.hpp"
#include "SharedObjManager.hpp"
#include "DefaultAllocate.h"

/* ---------------------------------------------------------
 *            Realtime Object Transfer Controller
 * ---------------------------------------------------------
 */
namespace libfcn_v2 {

    void singleWriteFrameBuilder(
            DataLinkFrame* result_frame,
            uint16_t src_id,
            uint16_t dest_id,
            uint16_t op_code,
            uint16_t msg_id,
            uint8_t* p_data, uint16_t len);


    /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
    obj_size_t RtoDictSingleWrite(ObjectDictMM* dict,
                                      obj_idx_t index,
                                      uint8_t *data, obj_size_t len);

    obj_size_t RtoBufferWrite(void* buffer,
                              obj_idx_t index,
                              uint8_t *data, obj_size_t len);

    class NetworkLayer;

    /* 共享字典管理器 */
    typedef SharedObjManager<ObjectDictMM> RtoDictManager;

    class PubSubChannel{
    public:
        PubSubChannel(ObjectDictMM* obj_dict_shm)
            :   obj_dict_shm(obj_dict_shm){ }

        ~PubSubChannel() = default;

        /* 是否为 "多源通道"。 */
        bool is_multi_source{false};

        /* 通道ID */
        int channel_addr{0};

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
            obj_dict_shm->write(msg);

            /* TODO: 进行本地发布的回调 */

            /* 再进行网络发布：
             * TODO：根据发布管理进行分频*/
            singleWriteFrameBuilder(
                    &frame_tmp,
                    src_id,
                    dest_id,
                    static_cast<uint8_t>(OpCode::RTO_PUB),
                    msg.index,
                    (uint8_t *)msg.getDataPtr(), msg.data_size);

            networkPublish(&frame_tmp);
        }

        template<typename Prototype>
        Prototype fetchBuffer(Prototype&& msg){
            auto p_buffer = obj_dict_shm->p_buffer;

            USER_ASSERT(p_buffer!= nullptr);

            Prototype res = msg;

            utils::memcpy(&res.data,
                          (uint8_t*)p_buffer +
                          msg.buffer_offset,
                          sizeof(res.data));

            return res;
        }

        ObjectDictMM* obj_dict_shm{nullptr};

        /* 将回调函数指针映射到int8整形时，基址偏移量
         * 采用指针形式，因为存储映射表的堆会根据添加的实例数量进行调整
         * 同时会更改基址的值。用指针指向堆所分配的基址，可实现同步更新*/
        uint16_t*  callback_mapped_ptr_base;

        /* 将回调函数指针映射到int8整形后的指针数组 */
        uint8_t** callback_mapped_ptr_list;

        /* 上述数组长度 */
        uint8_t* callback_mapped_ptr_num;

        DataLinkFrame frame_tmp;

        libfcn_v2::NetworkLayer *network_layer;

        void networkPublish(DataLinkFrame* frame);
    };

    #define MAX_PUB_CTRL_RULES 10

    /*
     * 网络处理
     * */
    class RtoNetworkHandler{
    public:
        RtoNetworkHandler(NetworkLayer* network)
            : network(network),
              dict_manager(MAX_LOCAL_NODE_NUM),
              pub_ctrl_rules(MAX_PUB_CTRL_RULES){ }

        virtual ~RtoNetworkHandler() = default;

        template<typename T_Dict>
        PubSubChannel* createChannel(uint16_t address){
            auto dict = dict_manager.create<T_Dict>(address);
            if(dict->p_buffer == nullptr){
                dict->p_buffer = new typename T_Dict::Buffer();
            }


            auto channel = new PubSubChannel(dict);
            channel->network_layer = network;
            channel->channel_addr = address;
            return channel;
        }

        template<typename T_Dict>
        PubSubChannel* createChannel(uint16_t address, void* static_buffer){
            auto dict = dict_manager.create<T_Dict>(address);
            dict->p_buffer = static_buffer;

            auto channel = new PubSubChannel(dict);
            channel->network_layer = network;
            channel->channel_addr = address;
            return channel;
        }

        void handleWrtie(DataLinkFrame* frame, uint16_t recv_port_id);


        struct PubCtrlRule{
            PubCtrlRule() : data_link_dev(MAX_COM_PORT_NUM){}

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
            utils::vector_s<FrameIODevice*> data_link_dev;

        private:
            uint32_t freq_divier{0};
            uint32_t freq_divier_cnt{0};

            uint32_t send_busy_cnt {0};
            friend class RtoNetworkHandler;

        };

        void addPubCtrlRule(PubCtrlRule& rule);

        void update();


    protected:
        NetworkLayer* const network{nullptr};

        RtoDictManager dict_manager;
        uint16_t poll_freq_hz{1000};

        utils::vector_s<PubCtrlRule> pub_ctrl_rules;

    };
}



#endif //LIBFCN_V2_REALTIMEOBJECT_HPP
