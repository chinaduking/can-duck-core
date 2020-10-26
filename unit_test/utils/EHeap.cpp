//
// Created by sdong on 2019/11/30.
//

#include "utils/ObjPool.hpp"
//#include "utils/ESharedPtr.hpp"
#include <array>
#include "TestUtils.hpp"

using namespace utils;

TEST(ObjPool, dec) {

    int cnt = 10;

    cout << "result: " << to_string(-- cnt ) << endl;
    cout << "result: " << to_string(cnt --) << endl;
}


TEST(ObjPool, StaticInit){
    ObjPool<int, 100> int_obj_pool;

    int* data = (int*)int_obj_pool.allocate();

    ASSERT_EQ(int_obj_pool.usage(), 1);

    int_obj_pool.deallocate(data);

    ASSERT_EQ(int_obj_pool.usage(), 0);
}

namespace obj_pool_test{

    ObjPool<int, 100> intObjPool;

    struct IntAllocator{
        static void* allocate(size_t size){
            return intObjPool.allocate();
        }

        static void deallocate(void* p){
            intObjPool.deallocate(p);
        }
    };



}

