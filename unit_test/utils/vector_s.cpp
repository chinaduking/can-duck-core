//
// Created by sdong on 2019/11/24.
//

#include "utils/vector_s.hpp"
#include <gtest/gtest.h>
#include <string>
#include <iostream>

using namespace utils;
using namespace std;

TEST(vector_s, push_pop) {
    vector_s<int> vec(10);
    for(int i = 0; i < 10; i ++){
        vec.push_back(i);
    }

    for(int i = 0; i < 10; i ++){
        ASSERT_EQ(i, vec[i]);
        cout << "i:" << vec[i] << endl;
    }

    while(!vec.empty()){
        cout << "i:" << vec.pop() << endl;
    }
}


TEST(vector_s, range_for) {
    vector_s<int> s(10);
    for(int i = 0; i < 7; i ++){
        s.push_back(i);
    }

    for(auto item : s){
        cout << "i:" <<item << endl;
    }

    for(auto& item : s){
        cout << "i:" <<item << endl;
    }
}