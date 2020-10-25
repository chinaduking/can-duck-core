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


    /* 数据长度标志位为无符号8位整形，最大255
     * */
    #define FCN_MAX_OBJ_SIZE 0xFF
    typedef uint8_t index_t;
    typedef uint8_t data_size_t;

#pragma pack(2)
    /*非阻塞式任务的回调函数*/
    struct FcnCallbackInterface{
        virtual void callback(void* data, uint8_t ev_code) = 0;
    };

    /*
     * 对象字典（Object Dictionary）成员.
     * 内存按2Byte对齐，更改时要注意, sizeof(ObjDictItemBase) = 2
     * */
    struct ObjDictItemBase {
        ObjDictItemBase(index_t index,
                        data_size_t data_size)
                :
                index(index),
                data_size(data_size){ }

        /* 消息索引 */
        const index_t index{0};

        /* 消息数据大小，最长255字节。不支持变长 */
        const data_size_t data_size{0};
    };


    /*
     * 对象字典
     * */
    class ObjectDict {
    public:
        explicit ObjectDict(index_t dict_size);
        virtual ~ObjectDict();

        /*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
        data_size_t singleWrite(index_t index, uint8_t *data, data_size_t len);

        /* 获取字典大小 */
        index_t getDictSize();

        /*根据参数表索引获取元信息。未找到则返回空指针。 */
        ObjDictItemBase *getObject(index_t index);

        /*默认字段
         * TODO: 版本校验？
         * 1. 可手动校验：使用快照指令
         * 2. 在网络配置阶段，使用专用协议读取*/
//        RTODictItem<uint32_t> version;

    protected:
        ObjDictItemBase **obj_dict  {nullptr };
        index_t dict_size { 0 };

        /* 自定义写入一个项目后的动作（回调/置标志位等） */
        virtual void writePostAction(index_t& index){};
    };



    /*
     * 对象字典（Object Dictionary）成员.
     * 内存按2Byte对齐，更改时要注意, sizeof(ObjDictItemBase) = 4
     * */
    struct RtoDictItemBase : public ObjDictItemBase{
        RtoDictItemBase(index_t index,
                        data_size_t data_size,
                        bool derived_has_callback=false)
                    :
            ObjDictItemBase(index, data_size),
            derived_has_callback(derived_has_callback)
        {
            timestamp = 0;
        }


        /* 子类字典成员是否含有回调对象（TransferCallbackPtr）的标志位
         * （为了节省函数指针的内存） */
        const uint16_t derived_has_callback : 1;

        /* 时间戳 */
        uint16_t timestamp          : 15;

        /*
         * 取得子类数据对象。无回调，则子类必须将数据放在第一个成员；有回调，则放在回调对象之后
         */
        inline uint8_t* getDataPtr(){
            if(!derived_has_callback){
                return ((uint8_t*)this) + sizeof(RtoDictItemBase);
            } else{
                return ((uint8_t*)this) + sizeof(RtoDictItemBase) +
                       sizeof(FcnCallbackInterface);
            }
        }

        /*
         * 取得子类回调对象（如果不支持回调则返回空指针）
         */
        inline FcnCallbackInterface* getCallbackPtr(){
            if(!derived_has_callback){
                return nullptr;
            } else{
                /* 由括号内向括号外：
                 * 1. 取得存储回调地址的指针的地址。
                 * 2. 按指针的数据类型解引用，取得指针所指的回调的地址。
                 * 3. 根据回调地址构造指向回调的指针，并返回。
                 * */
                return (FcnCallbackInterface*)(*(uint64_t*)(
                        (uint8_t*)this + sizeof(RtoDictItemBase)));
            }
        }
    };

    /*
     * 实时数据对象（Real-Time Object）字典成员
     * 实现了类型安全的数据存储。
     * */

    /*
     * 不支持回调的字典项目
     * */
    template <typename T>
    struct RtoDictItemNoCb : public RtoDictItemBase{
        explicit RtoDictItemNoCb(index_t index):
                RtoDictItemBase(index, sizeof(T), false){}

        void operator<<(T input) { data = input; }
        void operator>>(T &input) { input = data; }

        T data;
    };


    /*
     * 支持回调的字典项目
     * */
    template <typename T>
    struct RtoDictItemCb : public RtoDictItemBase{
        explicit RtoDictItemCb(index_t index):
                RtoDictItemBase(index, sizeof(T), true){}

        void operator<<(T input) { data = input; }
        void operator>>(T &input) { input = data; }

        FcnCallbackInterface* callback{nullptr};
        T data;
    };

#pragma pack(0)

}



/* ---------------------------------------------------------
 *            Realtime Object Transfer Controller
 * ---------------------------------------------------------
 */
namespace libfcn_v2 {

    void RtoFrameBuilder(
            DataLinkFrame* result_frame,
            ObjectDict* dict,
            index_t index);

    void RtoFrameBuilder(
            DataLinkFrame* result_frame,
            ObjectDict* dict,
            index_t index_start, index_t index_end);



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


        ObjectDict* getSharedDictByAddr(uint16_t address);

    private:
        RtoShmManager() : managed_items(MAX_LOCAL_NODE){}

        struct MaganedItem{
            uint32_t address {1000};
            ObjectDict* p_dict {nullptr};
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
            ObjectDict* dict{nullptr};
            index_t start_or_single_idx  {0};
            int end_idx    { -1 };

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
