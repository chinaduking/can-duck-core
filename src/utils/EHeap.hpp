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
    #define EHEAP_STATIC_INIT(instance, bloack_size, block_num) \
        uint8_t m_##instance##_memblock[bloack_size * block_num]; \
        uint8_t* m_##instance##_freestack[block_num]; \
        EHeap instance( \
                m_##instance##_memblock, m_##instance##_freestack, \
                bloack_size, block_num);

    class EHeap{
    public:
        EHeap(uint8_t* p_mem_block,
                uint8_t** p_free_stack,
              uint32_t bloack_size, uint32_t block_num);

        EHeap(uint32_t bloack_size, uint32_t block_num);
        ~EHeap();
        uint8_t *getMemBlock(uint32_t size);
        void  returnMemBlock(uint8_t* data);
        uint32_t getUsedBlock();

    private:

        uint8_t * block_ptr;
        uint8_t** free_block_stack;
        uint32_t  free_stack_top;
        uint32_t  max_block_num;
        uint32_t  bloack_size;
        bool is_static_alloc;
#ifdef SYSTYPE_FULL_OS
        std::mutex io_mutex;
#endif  //SYSTYPE_FULL_OS

    };

}


#endif //LIBFCN_EHEAP_HPP
