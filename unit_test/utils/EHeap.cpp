//
// Created by sdong on 2019/11/30.
//

#include "utils/EHeap.hpp"
#include "utils/ESharedPtr.hpp"
#include <gtest/gtest.h>
#include <string>
#include <iostream>

using namespace emros;
using namespace std;

TEST(EHeap, dec) {

    int cnt = 10;

    cout << "result: " << to_string(-- cnt ) << endl;
    cout << "result: " << to_string(cnt --) << endl;
}

namespace emros_test{
    EHeap frame_heap(64, 10);

    class TBase{
    public:
        virtual ~TBase(){
            cout<< "TBase destructor.. " << endl;
        }

        string name;

        void * operator new(size_t size) noexcept
        {
            cout<< "Overloading new operator with size: " << size << endl;
            void * p = frame_heap.getMemBlock(size);

            return p;
        }

        void operator delete(void * p)
        {
            cout<< "Overloading delete operator " << endl;
            frame_heap.returnMemBlock((uint8_t*)p);
        }
    };

    class TInherit : public TBase{
    public:
        virtual ~TInherit() override {
            cout<< "TInherit destructor.. " << endl;
        }


        int num;
    };

    TEST(EHeap, newDelete) {
        TBase* base = new TInherit();
        delete base;
    }

    TEST(EHeap, malloc) {
        TBase* queue[15];

        for(int i = 0; i < 15; i ++){
            queue[i] = new TInherit();

            if(queue[i] == nullptr){
                cout << "out of mem!!" << endl;
                continue;
            }

            ((TInherit*)queue[i])->num = i;
            cout << "push  " << to_string(i) << endl;
            cout << "TInherit::num = " << to_string(((TInherit*)queue[i])->num) << endl;
        }

        cout << "pushed..." << endl;

        for(int i = 0; i < 15; i ++){
            if(queue[i] != nullptr) {
                cout << "TInherit::num = " << to_string(((TInherit*)queue[i])->num) << endl;
            }
        }
    }

    TEST(EHeap, EAutoPtr) {
        ESharedPtr<TBase> queue[15];

        for(int i = 0; i < 15; i ++){
            ESharedPtr<TBase> mem(new TInherit());

            if(mem.isNull()){
                cout << "out of mem!!" << endl;
                continue;
            }

            queue[i] = mem;
            ((TInherit&)*queue[i]).num = i;
            cout << "push  " << to_string(i) << endl;
            cout << "TInherit::num = " << to_string(((TInherit&)*queue[i]).num) << endl;
        }

        cout << "pushed..." << endl;

        for(int i = 0; i < 15; i ++){
            if( !queue[i].isNull() ) {
                cout << "TInherit::num = " << to_string(((TInherit&)*queue[i]).num) << endl;
            }
        }
    }


    class ipv{
    public:
        virtual void func() = 0;
        virtual ~ipv(){
            cout << "~ipv" << endl;
        }
    };

    class pv : public ipv{
    public:
        void func()override{

        }
        ~pv()  override {
            cout << "~pv" << endl;
        }
    };

    TEST(EHeap, EAutoPtrVirtual) {
        ESharedPtr<ipv> sp(new pv);
    }


    TEST(EHeap, StaticInit) {
        EHEAP_STATIC_INIT(eheap, 1, 100);
        auto p = eheap.getMemBlock(1);
        eheap.returnMemBlock(p);
    }
}


