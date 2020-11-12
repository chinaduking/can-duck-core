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

    for(int i = 0; i < 13; i ++){
        q.push(i);
        cout << "push: q.front() = "<< q.front() << endl;
        cout << "push: q.back() = "<< q.back() << endl;
        cout << "emptyness:"<< q.empty() << endl;
        cout << "size:"<< q.size() << endl;
        for(int j = 0; j < q.size(); j ++){
            cout << q[j] << ", ";
        }
        cout << endl;
    }

    cout << "\n\n" << endl;


    for(int i = 0; i < 7; i ++){

        if(!q.empty()){
            cout << "pop: q.front() = "<< q.front() << endl;
            cout << "push: q.back() = "<< q.back() << endl;

            for(int j = 0; j < q.size(); j ++){
                cout << q[j] << ", ";
            }
            cout << endl;

            q.pop();
        }
        else{
            cout << "empty!" << endl;
        }
    }

    cout << "\n\n" << endl;


    for(int i = 0; i < 7; i ++){
        q.push(i);
//        cout << "push: q.front() = "<< q.front() << endl;
//        cout << "push: q.back() = "<< q.back() << endl;
        cout << "emptyness:"<< q.empty() << endl;
        cout << "size:"<< q.size() << endl;
        for(int j = 0; j < q.size(); j ++){
            cout << q[j] << ", ";
        }
        cout << endl;
    }

    cout << "\n\n" << endl;

    for(int i = 0; i < 11; i ++){

        if(!q.empty()){
            cout << "pop: q.front() = "<< q.front() << endl;
            cout << "push: q.back() = "<< q.back() << endl;

            for(int j = 0; j < q.size(); j ++){
                cout << q[j] << ", ";
            }
            cout << endl;

            q.pop();
        }
        else{
            cout << "empty!" << endl;
        }
    }
}

//struct C{
//    int a = 0;
//    ~C(){
//        cout << "C: dtor, a = " << a << endl;
//    }
//};
//
//TEST(queue, des){
//
//    queue_s<C> q(5);
//
//    for(int i = 0; i < 13; i ++){
//        C c;
//        c.a = i;
//        cout << "pushing" << endl;
//        q.push(std::move(c));
//    }
//
//    cout << "\n\n" << endl;
//
//
//    for(int i = 0; i < 7; i ++){
//        cout << "pop" << endl;
//
//        if(!q.empty()){
//            q.pop();
//        }
//    }
//}




