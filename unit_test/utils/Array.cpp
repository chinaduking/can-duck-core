//
// Created by sdong on 2019/11/24.
//

#include "utils/Array.hpp"

#include <gtest/gtest.h>
#include <iostream>

using namespace utils;
using namespace std;

TEST(Array, test) {
    Array<int, 10> array;
    array[3] = 5;

    cout << "\narray size:" << sizeof(array) << endl;
    cout << array[3] << endl;

    auto array2 = array;

    cout << array2[3] << endl;

}