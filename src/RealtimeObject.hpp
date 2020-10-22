//
// Created by sdong on 2020/10/15.
//

#ifndef LIBFCN_V2_REALTIMEOBJECT_HPP
#define LIBFCN_V2_REALTIMEOBJECT_HPP

#include <cstdint>
#include "utils/vector_s.hpp"
#include "DataLinkLayer.hpp"

#ifdef SYSTYPE_FULL_OS
#include <functional>
#endif

/* ---------------------------------------------------------
 *               Realtime Object Definition
 * ---------------------------------------------------------
 */
namespace libfcn_v2 {

#pragma pack(2)

    /*
     *
     * */
    struct RealtimeObjectBase {
        RealtimeObjectBase(uint8_t index, uint8_t data_size) :
                index(index), data_size(data_size) {}

        uint16_t index;
        uint16_t data_size;

        inline uint8_t* getDataPtr(){
            return ((uint8_t*)this) + sizeof(RealtimeObjectBase);
        }

#ifdef SYSTYPE_FULL_OS
        std::function<void(void *)> *callback;
#endif
    };


    /*
     *
     * */
    template<class T>
    struct RealtimeObject : public RealtimeObjectBase {

        RealtimeObject() : RealtimeObjectBase(0, sizeof(T)) {}

        void operator<<(T input) { data = input; }
        void operator>>(T &input) { input = data; }

        T data;
    };

#pragma pack(0)


    /*
     *
     * */
    #define MAX_LOCAL_NODE 6

    class RealtimeObjectDict {
    public:
        explicit RealtimeObjectDict(uint16_t dict_size);

        virtual ~RealtimeObjectDict() = default;


        /*从参数表读取1个项目至缓冲区。限制最大长度。返回实际长度。超长返回0*/
        uint16_t read(uint16_t index, uint8_t *data, uint16_t len);

        /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
        uint16_t singleWrite(uint16_t index, uint8_t *data, uint16_t len);

        uint32_t size();

        /*根据参数表索引获取元信息。未找到则返回空指针。 */
        RealtimeObjectBase *getObject(uint16_t index);


        /*默认字段
         * TODO: 版本校验？
         * 1. 可手动校验：使用快照指令
         * 2. 在网络配置阶段，使用专用协议读取*/
        RealtimeObject<uint32_t> version;

    protected:
        RealtimeObjectBase **obj_dict{nullptr};

        uint16_t dict_size;
    };


    void RtoFrameBuilder(
            DataLinkFrame* result_frame,
            RealtimeObjectDict* dict,
            uint16_t index);

    void RtoFrameBuilder(
            DataLinkFrame* result_frame,
            RealtimeObjectDict* dict,
            uint16_t index_start, uint16_t index_end);
}



/* ---------------------------------------------------------
 *            Realtime Object Transfer Controller
 * ---------------------------------------------------------
 */
namespace libfcn_v2 {

    #define MAX_LOCAL_NODE 6

    class RtoNetworkHandler;

    /*
     * 共享内存管理器
     * 对于同一地址，RTOD实例是单例的，返回指针。
     * */
    class RtoShmManager {
    public:
        friend class RtoNetworkHandler;

        static RtoShmManager* getInstance();

        virtual ~RtoShmManager() = default;

        template<typename T_Dict>
        T_Dict* getSharedDict(uint16_t address){

            /* if we can find an exsiting shm, return it */
            for(auto & managed_item : managed_items){
                if(managed_item.address == address){
                    return (T_Dict*)(managed_item.p_dict);
                }
            }

            /* else, create a new one */
            MaganedItem item = {
                    .address = address,
                    .p_dict  = new T_Dict()
            };

            managed_items.push_back(item);
            return (T_Dict*)(item.p_dict);
        }


        RealtimeObjectDict* getSharedDictByAddr(uint16_t address);

    private:
        RtoShmManager() : managed_items(MAX_LOCAL_NODE){}

        struct MaganedItem{
            uint32_t address {1000};
            RealtimeObjectDict* p_dict {nullptr};
        };

        utils::vector_s<MaganedItem> managed_items;

        static RtoShmManager* instance;
    };


    #define MAX_PUB_CTRL_RULES 10

    /*
     * 网络处理
     * */
    class RtoNetworkHandler{
    public:
        RtoNetworkHandler(RtoShmManager* rto_manager, uint16_t poll_freq_hz)
            : rto_manager(rto_manager),
            poll_freq_hz(poll_freq_hz) ,
            pub_ctrl_rules(MAX_PUB_CTRL_RULES){ }

        virtual ~RtoNetworkHandler() = default;

        template<typename T_Dict>
        T_Dict* bindDictToChannel(uint16_t address){
            return rto_manager->getSharedDict<T_Dict>(address);
        }

        void handleWrtie(DataLinkFrame* frame);


        struct PubCtrlRule{
            PubCtrlRule() : data_link_dev(MAX_COM_PORT_NUM){}
            uint16_t freq_hz     { 100 };
            uint16_t src_address {  0  };
            uint16_t dest_address{  0  };

            /*
             * end_idx != 0xFFFF : start_idx
             * end_idx == 0xFFFF : single_idx
             **/
            RealtimeObjectDict* dict;
            uint16_t start_or_single_idx  {0xFFFF};
            uint16_t end_idx    {0xFFFF};

            /* TODO: Random write mode? Is it necessary? */
            //utils::StaticSet<uint16_t> rto;

        private:
            uint32_t freq_divier{0};
            uint32_t freq_divier_cnt{0};

            uint32_t send_busy_cnt {0};
            friend class RtoNetworkHandler;

            utils::vector_s<FrameIODevice*> data_link_dev;
        };

        void addPubCtrlRule(PubCtrlRule& rule);

        void update();


    protected:
        RtoShmManager* rto_manager;
        uint16_t poll_freq_hz{1000};

        utils::vector_s<PubCtrlRule> pub_ctrl_rules;
    };
}



#endif //LIBFCN_V2_REALTIMEOBJECT_HPP
