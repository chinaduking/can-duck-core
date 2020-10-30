//
// Created by sdong on 2019/11/8.
//

#include "utils/CppUtils.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace utils;
using namespace std;

TEST(CppUtils, existInVector) {
    std::vector<int> x(3);
    x[0] = 1;
    x[1] = 3;
    x[2] = -1;

    cout << "\nresult =" <<
        existInVector<int>(x, 3) << endl;
    cout << "result =" <<
         existInVector<int>(x, 0) << endl;
}

TEST(CppUtils, perciseSleep) {

    for(int i = 0; i < 100; i ++){
        cout << i << endl;
        perciseSleep(0.1);
    }

}

TEST(CppUtils, memcpy) {

    uint8_t src[7], dest[7];

    for(int i = 0; i < 7; i ++){
        src[i] = i;
    }

    utils::memcpy(dest, src, 7);

    for(int i = 0; i < 7; i ++){
        cout<< std::to_string(dest[i]) << ",";
    }
    cout << endl;

    ASSERT_EQ(memcmp(src, dest, 7), 0);
}
