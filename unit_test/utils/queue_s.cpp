//
// Created by sdong on 2020/11/7.
//

#include "queue_s.hpp"
#include <gtest/gtest.h>
#include <string>
#include <iostream>

using namespace utils;
using namespace std;

TEST(queue, io){

    queue_s<int> q(5);

    for(int i = 0; i < 10; i ++){
        q.push(i);
        cout << "push: q.front() = "<< q.front() << endl;

        for(int j = 0; j < q.size(); j ++){
            cout << q[j] << ", ";
        }
        cout << endl;
    }


    for(int i = 0; i < 7; i ++){
        q.pop();
        if(!q.empty())
            cout << "pop: q.front() = "<< q.front() << endl;
        else
            cout << "empty!" << endl;

//        if(!q.empty()){
//            for(int j = 0; j < q.size(); j ++){
//                cout << q[j] << ", ";
//            }
//            cout << endl;
//        } else{
//            cout << "empty!" << endl;
//        }

    }
//    for(int i = 0; i < 7; i ++){
//        q.push(i);
//
//        for(int j = 0; j < q.size(); j ++){
//            cout << q[j] << ", ";
//        }
//        cout << endl;
//    }
//
//
//    for(int i = 0; i < 7; i ++){
//        q.pop();
//
//        if(!q.empty()){
//            for(int j = 0; j < q.size(); j ++){
//                cout << q[j] << ", ";
//            }
//            cout << endl;
//        } else{
//            cout << "empty!" << endl;
//        }
//
//    }


}



