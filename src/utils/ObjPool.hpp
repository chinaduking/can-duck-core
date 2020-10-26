//
// Created by sdong on 2019/11/30.
//

#ifndef LIBFCN_EHEAP_HPP
#define LIBFCN_EHEAP_HPP

#include <cstdint>

#ifdef SYSTYPE_FULL_OS
#include <mutex>
#endif  //SYSTYPE_FULL_OS

/*
 * Eheap = Embedded Heap
 *
 * 嵌入式系统使用的无碎片快速堆。
 *
 * 分配和释放均为O(1)复杂度，即不论堆总大小和已经分配的大小，
 * 进行分配释放操作时均只消耗几个指令周期。
 *
 * EHeap维护一组内存区块和一个指针栈，栈中每一个指针均指向
 * 一块未被分配的内存。分配时从指针栈弹出一个指针，释放时
 * 将指针压栈。
 *
 * EHeap只能实现固定大小的内存分配，且释放时指针必须正确归还
 * 到相对应的栈。这可以通过重载特定对象的new和delete操作符，
 * 使之绑定到栈实例而实现。
 *
 * 这种方式通过适当地牺牲灵活性，消除了反复调用常规的
 * malloc/free造成的内存碎片，且速度要快得多。
 *
 * 适合用于对应能要求较高的特定算法。
 * */

namespace utils{
    typedef uint32_t eheap_size_t;

    template<typename T, uint32_t N>
    class ObjPool{
    public:
        ObjPool() {
            init();
        };

        ~ObjPool() = default;

        void init(){
            for(int i = 0; i < N; i ++){
                obj_pool[i] = T();
                free_block_stack[i] = &obj_pool[i];
            }
        }

        void* allocate(){
#ifdef SYSTYPE_FULL_OS
            std::lock_guard<std::mutex> lock_guard(io_mutex);
#endif
            /* 栈已空 */
            if(free_stack_top == N){
                return nullptr;
            }

            /* 从栈顶弹出一个元素 */
            return free_block_stack[free_stack_top++];
        }


        void  deallocate(void* data){
#ifdef SYSTYPE_FULL_OS
            std::lock_guard<std::mutex> lock_guard(io_mutex);
#endif
            /* 已经被安全释放的数据不会再次释放，否则队列会被null填充 */
            if(data == nullptr){
                return;
            }

            /* 避免栈指针从栈底越界
             * （常因为对应了错误的returnMemBlock，或重复调用了delete） */
            if(free_stack_top > 0){
                free_stack_top --;
                free_block_stack[free_stack_top] = (T*)data;
            }
        }


        uint32_t usage(){
            return free_stack_top;
        }

        const eheap_size_t capicity  {N};

    private:

        T  obj_pool[N];
        T* free_block_stack[N];

        eheap_size_t free_stack_top {0};

#ifdef SYSTYPE_FULL_OS
        std::mutex io_mutex;
#endif  //SYSTYPE_FULL_OS

    };

}


#endif //LIBFCN_EHEAP_HPP
