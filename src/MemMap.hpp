//
// Created by sdong on 2020/10/28.
//

#ifndef LIBFCN_V2_MEMMAP_HPP
#define LIBFCN_V2_MEMMAP_HPP

namespace libfcn_v2{

    /* 将小范围的指针映射到大范围 */
    class MemMap{
    public:
        MemMap(int n):
                capicity(n){
            mmap = new void*[n];
            for(int i = 0; i < n; i ++){
                mmap[i] = nullptr;
            }
        }

        ~MemMap(){
            delete [] mmap;
        }

        inline int add(void* ptr){
            if(size >= capicity){
                return -1;
            }

            mmap[size] = ptr;
            size ++ ;
            return size - 1;
        }

        inline void* query(int v_ptr){
            if(v_ptr >= size){
                return nullptr;
            }

            return mmap[v_ptr];
        }

    private:
        void** mmap;

        int size {0};
        int capicity{ 0 };
    };

    template <typename T, typename Vptr_T, typename MemMap>
    struct vptr{
        vptr(T* ptr){
            id = MemMap::add(ptr);
        }

        /* 重载解引用操作，获取智能指针指向的数据实例 */
        T& operator*() {
            return *((T*)MemMap::query(id));
        }

        /* 重载成员访问操作，获取智能指针指向的数据实例的成员 */
        T* operator->() {
            return (T*)MemMap::query(id);
        }

        Vptr_T id{0};
    };

}


#endif //LIBFCN_V2_MEMMAP_HPP
