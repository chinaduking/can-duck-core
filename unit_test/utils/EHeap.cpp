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

    int* data = int_obj_pool.allocate();

    ASSERT_EQ(int_obj_pool.usage(), 1);

    int_obj_pool.deallocate(data);

    ASSERT_EQ(int_obj_pool.usage(), 0);
}


