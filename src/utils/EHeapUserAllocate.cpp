//
// Created by sdong on 2020/10/21.

#include "utils/EHeap.hpp"
#include "utils/ESharedPtr.hpp"

namespace utils{
    #define FCN_ALLOCATE_FRAME_NUM 20

    EHEAP_STATIC_INIT(
            g_RefCntHeap,
    sizeof(ESharedPtr<int>::RefCount),
            FCN_ALLOCATE_FRAME_NUM)

//    EHEAP_STATIC_INIT(
//            g_ShareMQNodeHeap,
//    sizeof(SharedMQ<int>::Node),
//            FCN_ALLOCATE_FRAME_NUM)

//    EHEAP_STATIC_INIT(
//            g_EvloopTaskHeap,
//    100,
//    10)
}
