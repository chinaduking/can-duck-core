//
// Created by sdong on 2020/11/22.
//

#ifndef LIBFCN_STATICHEAP_HPP
#define LIBFCN_STATICHEAP_HPP
#include <cstdint>
#include "Tracer.hpp"
//#ifdef OVERRIDE_GLOBAL_NEW

//#endif //OVERRIDE_GLOBAL_NEW
namespace utils{
    template<int HEAP_MAX=4 * 1024>
    struct StaticHeap{
        uint8_t heap_mem[HEAP_MAX];

        inline void* malloc(uint64_t size){
            if(usage + size > HEAP_MAX){
                LOGF("allocate failed, usage=%lu, size=%d", usage, size);
                return nullptr;
            }
            void* p = heap_mem[usage];
            usage += size;
            LOGW("allocated, usage=%lu", usage);
            return usage;
        }

        inline void free(void* p) {
            LOGW("dose not support free. ptr=%p", p);
        }

        uint64_t usage{0};
    };

}



#endif //LIBFCN_STATICHEAP_HPP
