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

    void coutinuousWriteFrameBuilder(
            DataLinkFrame* result_frame,
            ObjectDictMM* dict,
            obj_idx_t index_start, obj_idx_t index_end,
            uint16_t src_id,
            uint16_t dest_id,
            uint16_t op_code);


    /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
    obj_size_t RtoDictContinuousWrite(ObjectDictMM* dict,
                                      obj_idx_t index,
                                      uint8_t *data, obj_size_t len);

    obj_size_t RtoBufferWrite(void* buffer,
                              obj_idx_t index,
                              uint8_t *data, obj_size_t len);

    class NetworkLayer;

    /* 共享字典管理器 */
    typedef SharedObjManager<ObjectDictMM*> RtoDictManager;

    class PubSubChannel{
    public:
        PubSubChannel(ObjectDictMM* obj_dict_shm)
            :   obj_dict_shm(obj_dict_shm){ }

        ~PubSubChannel() = default;

        /* 是否为 "多源通道"。 */
        bool is_multi_source;

        /* 通道ID */
        int channel_addr{0};

        template<typename Msg>
        void publish(Msg&& msg){

        }

        template<typename Msg>
        Msg fetchBuffer(Msg&& msg){
            Msg res;

            utils::memcpy(&res.data,
                          obj_dict_shm->p_buffer + msg.buffer_offest,
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
            auto p_buffer = dict_manager.create<T_Dict::Buffer>(address);
            auto channel = new PubSubChannel(p_buffer);
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
