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
            RealtimeObjectDict* dict,
            obj_idx_t index_start, obj_idx_t index_end,
            uint16_t src_id,
            uint16_t dest_id,
            uint16_t op_code);


    /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
    obj_size_t RtoDictContinuousWrite(RealtimeObjectDict* dict,
                               obj_idx_t index,
                               uint8_t *data, obj_size_t len);

    class NetworkLayer;

    /* 共享字典管理器 */
    typedef SharedObjManager<RealtimeObjectDict> RtoDictManager;

    class PubSubChannel{
    public:
        PubSubChannel() = default;
        ~PubSubChannel() = default;

        /* 是否为 "多源通道"。 */
        bool is_multi_source;

        /* 通道ID */
        int channel_addr{0};

        void publish(RealtimeObjectBase& msg);

        void fetchBuffer(RealtimeObjectBase& msg);
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
        T_Dict* bindDictToChannel(uint16_t address){
            return dict_manager.create<T_Dict>(address);
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
