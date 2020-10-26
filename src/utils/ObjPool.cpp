//
// Created by sdong on 2019/12/1.
//

#include "ObjPool.hpp"
#include <iostream>
using namespace utils;

#if 0
EHeap::EHeap( uint8_t* p_mem_block, uint8_t** p_free_stack,
        uint32_t bloack_size, uint32_t block_num):

        bloack_size(bloack_size),
        max_block_num(block_num),
        free_stack_top(0){

    block_ptr = p_mem_block;
    free_block_stack = p_free_stack;
    for (int i = 0; i < block_num; i++) {
        free_block_stack[i] = block_ptr + i*bloack_size;
    }

    is_static_alloc = true;
}


EHeap::EHeap(uint32_t bloack_size, uint32_t block_num) :
        bloack_size(bloack_size),
        max_block_num(block_num),
        free_stack_top(0){

    block_ptr = new uint8_t[bloack_size * block_num];
    free_block_stack = new uint8_t*[block_num];
    for (int i = 0; i < block_num; i++) {
        free_block_stack[i] = block_ptr + i*bloack_size;
    }

    is_static_alloc = false;
}

EHeap::~EHeap(){
    if(!is_static_alloc){
        delete [] block_ptr;
        delete [] free_block_stack;
    }
}

uint8_t * EHeap::getMemBlock(uint32_t size){
#ifdef SYSTYPE_FULL_OS
    std::lock_guard<std::mutex> lock_guard(io_mutex);
#endif

    /* 一个区块空间不足，直接判定失败 */
    if(size > bloack_size){
        return nullptr;
    }

    /* 栈已空 */
    if(free_stack_top == max_block_num){
        /* TODO: trace.. */
        std::cout << "-->>> heap overflow: " << std::endl;

        return nullptr;
    }

    /* 从栈顶弹出一个元素 */
    return free_block_stack[free_stack_top++];
}

void  EHeap::returnMemBlock(uint8_t* data){
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
        free_block_stack[free_stack_top] = data;
    } else{
        std::cout << "-->>> heap release under flow: " << data << std::endl;
        //TODO: trace or set error code!!
    }
}


uint32_t EHeap::getUsedBlock(){
    return free_stack_top;
}

#endif