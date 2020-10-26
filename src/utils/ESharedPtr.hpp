//
// Created by sdong on 2019/11/30.
//

#ifndef LIBFCN_ESHAREDPTR_HPP
#define LIBFCN_ESHAREDPTR_HPP

#include <cstdint>
#include "ObjPool.hpp"

#ifdef SYSTYPE_FULL_OS
#include <mutex>
#endif

namespace utils{
    extern ObjPool g_RefCntHeap;

    template <typename T>
    class ESharedPtr
    {

    public:
        class RefCount{
        public:
            uint32_t ref_cnt;

            /*TODO:
             * 1. 上位机直接使用包装后的std::shared_ptr即可。互斥锁性能较低
             * 2. 下位机可以检测竞争，并设置错误码，但并不实现互斥锁。
             *    因为一般整个FCN协议栈应该工作在同一线程。
             * */
#ifdef SYSTYPE_FULL_OS
#define REFCOUNT_LOCK_GUARD std::lock_guard<std::mutex> lk(io_mutex)
            std::mutex io_mutex;
#else
#define REFCOUNT_LOCK_GUARD do{}while(0)
#endif

            T* p_data;

            /* TODO: overload new/delete to use EHeap */
            RefCount(T* p_data = nullptr) :
                    ref_cnt(0), p_data(p_data){}

            ~RefCount() = default;

            void inc(){
                REFCOUNT_LOCK_GUARD;

                ++ ref_cnt;
            }
            uint32_t release(){
                REFCOUNT_LOCK_GUARD;

                ref_cnt --;
                if(ref_cnt <= 0){
                    delete p_data;
                    p_data = nullptr;
                }

                return ref_cnt;
            }

            uint32_t get(){
                REFCOUNT_LOCK_GUARD;
                return ref_cnt;
            }

            void * operator new(size_t size) noexcept {
                return g_RefCntHeap.allocate(size);
            }

            void operator delete(void * p) {
                g_RefCntHeap.deallocate((uint8_t *) p);
            }
        };

        RefCount* p_ref_cnt;

    public:

        /* 默认构造器，相当于构造了一个"空指针"，
         * 在没有被再次赋值之前，调用isNull返回true*/
        ESharedPtr():
                p_ref_cnt(nullptr){}


        /* 新实例构造器，
         * 从堆中构造一个新智能指针，且指向分配好的地址。
         * 注意：以智能指针的使用规范，object应该在传参的同时构造，
         * 不应先构造再传参。*/
        ESharedPtr(T* object){
            p_ref_cnt = new RefCount(object);
            p_ref_cnt->inc();
        }


        /* 拷贝构造器，
         * 从另一个智能指针实例构造新智能指针 */
        ESharedPtr(const ESharedPtr<T>& other)
                : p_ref_cnt(other.p_ref_cnt){
            p_ref_cnt->inc();
        }

        /* 析构，引用计数-1。引用计数为0时才执行真正的析构 */
        ~ESharedPtr() {
            if(p_ref_cnt == nullptr){
                return;
            }

            int cnt = p_ref_cnt->release();

            if (cnt <= 0){
                delete p_ref_cnt;
                p_ref_cnt = nullptr;
            }
        }

        bool isNull() const{
            return p_ref_cnt == nullptr;
        }

        uint32_t refCount(){
            if(p_ref_cnt == nullptr){
                return 0;
            }
            return p_ref_cnt->get();
        }

        /* 智能指针赋值。
         *
         * 关键点：如果this != other，即指向的目标变化，
         * 则需将原来指向目标的引用计数-1（减至0则删除）
         * 同时将新数据进行拷贝。
         * */
        ESharedPtr<T>& operator=(const ESharedPtr<T>& other){
            if (this == &other) {
                return *this;
            }

            if(p_ref_cnt != nullptr && p_ref_cnt->release() == 0){
                delete p_ref_cnt;
            }

            p_ref_cnt = other.p_ref_cnt;
            p_ref_cnt->inc();

            return *this;
        }

        /* 重载解引用操作，获取智能指针指向的数据实例 */
        T& operator*() {
            return *p_ref_cnt->p_data;
        }

        /* 重载成员访问操作，获取智能指针指向的数据实例的成员 */
        T* operator->() {
            return p_ref_cnt->p_data;
        }
    };

    template< class T, class... Args >
    ESharedPtr<T> makeESharedPtr( Args&&... args ){
        ESharedPtr<T> ptr(new T(args...));
        return ptr;
    }


}


#endif //LIBFCN_ESHAREDPTR_HPP
