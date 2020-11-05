//
// Created by sdong on 2019/11/30.
//

#include "utils/ObjPool.hpp"
//#include "utils/ESharedPtr.hpp"
#include <array>
#include "libfcn/TestUtils.hpp"

using namespace utils;

TEST(ObjPool, dec) {

    int cnt = 10;

    cout << "result: " << to_string(-- cnt ) << endl;
    cout << "result: " << to_string(cnt --) << endl;
}


TEST(ObjPool, alloc){
    ObjPool<int, 2> int_obj_pool;

    int* data_0 = (int*)int_obj_pool.allocate();
    ASSERT_EQ(int_obj_pool.usage(), 1);

    int* data_1 = (int*)int_obj_pool.allocate();
    ASSERT_EQ(int_obj_pool.usage(), 2);

    ASSERT_EQ((size_t)(data_1 - data_0), 1);

    ASSERT_EQ((size_t)((uint8_t *)data_1 - (uint8_t *)data_0),
              sizeof(int));

    int* data_2 = (int*)int_obj_pool.allocate();


    ASSERT_EQ(data_2, nullptr);

    ASSERT_EQ(int_obj_pool.usage(), 2);



    int_obj_pool.deallocate(data_0);

    data_0 = nullptr;

    int_obj_pool.deallocate(data_0);
    ASSERT_EQ(int_obj_pool.usage(), 1);


    int_obj_pool.deallocate(data_1);
    ASSERT_EQ(int_obj_pool.usage(), 0);
//    ASSERT_EQ(int_obj_pool.usage(), 0);
}

namespace obj_pool_test{

    ObjPool<int, 1> intObjPool;

    struct IntAllocator{
        static void* allocate(size_t size){
            return intObjPool.allocate();
        }

        static void deallocate(void* p){
            intObjPool.deallocate(p);
        }
    };

    TEST(ObjPool, StaticInit){
        int* data = (int*)IntAllocator::allocate(0);

        ASSERT_EQ(intObjPool.usage(), 1);

        IntAllocator::deallocate(data);

        ASSERT_EQ(intObjPool.usage(), 0);
    }
}

