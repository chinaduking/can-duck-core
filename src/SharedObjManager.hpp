//
// Created by sdong on 2020/10/27.
//

#ifndef LIBFCN_V2_SHAREDMEMMANAGER_HPP
#define LIBFCN_V2_SHAREDMEMMANAGER_HPP

#include "utils/vector_s.hpp"


namespace libfcn_v2{

    /*
     * 共享内存管理器
     * 对于同一地址，RTOD实例是单例的，返回指针。
     * */
    template<typename Base>
    class SharedObjManager{
    public:
        SharedObjManager(int n) : shared_objects(n){}

        ~SharedObjManager() = default;

        /*
         * 如果能找到已经存在的共享内存对象，返回它。
         * 否则添加ID记录并创建新对象。
         * if we can find an exsiting shm, return it。
         * else, create a new one  */
        template<typename Inherit>
        Inherit* create(uint16_t id){
            for(auto & obj : shared_objects){
                if(obj.id == id){
                    return (Inherit*)(obj.p_obj);
                }
            }

            SharedObj item = {
                .id = id,
                .p_obj   = new Inherit(),
                .mem_id = shared_objects.size()
            };

            shared_objects.push_back(item);
            return (Inherit*)(item.p_obj);
        }

        /*
         * 搜索共享对象，如果不存在则返回空指针
         * */
        Base* find(uint16_t id){
            for(auto & obj : shared_objects){
                if(obj.id == id){
                    return (Base*)(obj.p_obj);
                }
            }
            return nullptr;
        }


        int getMemoryID(uint16_t id){
            for(auto & obj : shared_objects){
                if(obj.id == id){
                    return obj.mem_id;
                }
            }
            return -1;
        }

        Base* findByMemID(int id){
            if(id >= shared_objects.size()
            || id < 0){
                return nullptr;
            }

            return shared_objects[id];
        }

    private:
        struct SharedObj{
            uint32_t id {1000};
            Base*    p_obj {nullptr};
            uint32_t mem_id;
        };

        utils::vector_s<SharedObj> shared_objects;
    };


}

#endif //LIBFCN_V2_SHAREDMEMMANAGER_HPP
