//
// Created by sdong on 2020/10/26.
//

#include "TestUtils.hpp"

#include "utils/LinkedList.hpp"

using namespace utils;

ObjPool<LinkedList<int>::Node, 20> listNodeObjPool;

struct LinkedListAllocator{
    static void* allocate(size_t size){
        return listNodeObjPool.allocate();
    }

    static void deallocate(void* p){
        listNodeObjPool.deallocate(p);
    }
};

TEST(LinkedList, io){

    LinkedList<int, LinkedListAllocator> int_list;

    int_list.push(5);
    ASSERT_EQ(int_list.size(), 1);
    ASSERT_EQ(int_list.tail(), 5);

    ASSERT_EQ(listNodeObjPool.usage(), 1);


    int data = 10;
    int_list.push(data);
    cout << "after push:  " << data << endl;

    ASSERT_EQ(int_list.size(), 2);
    ASSERT_EQ(int_list.tail(), 10);

    ASSERT_EQ(listNodeObjPool.usage(), 2);



    for(auto& i : int_list){
        cout << "data:  " << i << endl;
    }

    int_list.pop();

    for(auto& i : int_list){
        cout << "data:  " << i << endl;
    }

    ASSERT_EQ(listNodeObjPool.usage(), 1);

}

