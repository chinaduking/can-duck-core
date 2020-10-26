//
// Created by sdong on 2019/12/1.
//

#include "utils/ESharedPtr.hpp"
#include "utils/EHeap.hpp"
#include "utils/LinkedList.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "DataLinkLayer.hpp"
#include "FrameUtils.hpp"

using namespace utils;
using namespace libfcn_v2;
using namespace std;

extern EHeap m_FrameHeap;

namespace emros_test{
    void AquirePtrRefrence(ESharedPtr<DataLinkFrame>& p_frame){

        cout << "AquirePtrRefrence:: refcnt before local ptr assign: "
            << p_frame.refCount() << endl;
        ESharedPtr<DataLinkFrame> p_;
        p_ = p_frame;
        cout << "AquirePtrRefrence:: refcnt after local ptr assign: "
            << p_.refCount() << endl;


        p_frame = p_;
        p_->src_id += 10;
    }


    void AquirePtrCopy(ESharedPtr<DataLinkFrame> p_frame){

        cout << "AquirePtrCopy:: refcnt before local ptr assign: "
            << p_frame.refCount() << endl;
        ESharedPtr<DataLinkFrame> p_;
        p_ = p_frame;
        cout << "AquirePtrCopy:: refcnt after local ptr assign: "
            << p_.refCount() << endl;


        p_frame = p_;
        p_->src_id += 10;
    }


    TEST(AutoPtr, test) {
        ESharedPtr<DataLinkFrame> p_frame(new DataLinkFrame());
        cout << "refcnt after created: " << p_frame.refCount() << endl;

        cout << "\n\nrefcnt before AquirePtrRefrence: "
            << p_frame.refCount() << endl;
        ASSERT_EQ(p_frame.refCount(), 1);
        AquirePtrRefrence(p_frame);

        cout << "refcnt after AquirePtrRefrence: "
            << p_frame.refCount() << endl;
        ASSERT_EQ(p_frame.refCount(), 1);


        cout << "\n\nrefcnt before AquirePtrCopy: "
            << p_frame.refCount() << endl;
        ASSERT_EQ(p_frame.refCount(), 1);
        AquirePtrCopy(p_frame);
        cout << "refcnt after AquirePtrCopy: "
            << p_frame.refCount() << endl;
        ASSERT_EQ(p_frame.refCount(), 1);


        cout << "\n\n" << DataLinkFrameToString(*p_frame);
        ASSERT_EQ(m_FrameHeap.getUsedBlock(), 1);

    }

    TEST(AutoPtr, FrameVector) {
        {
            vector<ESharedPtr<DataLinkFrame>> frame_queue;

            for(int i = 0; i < 3; i++){
                frame_queue.emplace_back(new DataLinkFrame());
                frame_queue[i]->msg_id = i;
            }
            ASSERT_EQ(m_FrameHeap.getUsedBlock(), 3);

            auto frame_0 = frame_queue[0];
            ASSERT_EQ(frame_queue[0].refCount(), 2);
            ASSERT_EQ(frame_0.refCount(), 2);


            for(int i = 0; i < 3; i++){
                frame_queue.erase(frame_queue.begin());
                ASSERT_EQ(m_FrameHeap.getUsedBlock(), 2-i+1);
            }

            frame_0->dest_id = 1;
            cout << DataLinkFrameToString(*frame_0) << endl;
        }
        ASSERT_EQ(m_FrameHeap.getUsedBlock(), 0);
    }


    TEST(AutoPtr, FrameLinkedList) {
        LinkedList<ESharedPtr<DataLinkFrame>> frame_queue;

        for(int i = 0; i < 3; i++){
            ESharedPtr<DataLinkFrame> data(new DataLinkFrame());
            data->msg_id = i;
            frame_queue.append(data);
        }

        auto frame_0 = frame_queue.head()->val;


        for(int i = 0; i < 3; i++){
            frame_queue.popHead();
        }

        frame_0->msg_id = 10;
        cout << DataLinkFrameToString(*frame_0) << endl;
    }

    class EpopTest{
    public:
        ESharedPtr<DataLinkFrame>* working_frame_p{nullptr};
        thread* working_thread;
        void takeFromQueue(LinkedList<ESharedPtr<DataLinkFrame>>& queue){
            if(working_frame_p != nullptr){
                delete working_frame_p;
            }

            working_frame_p = new ESharedPtr<DataLinkFrame>(queue.head()->val);
//            queue.popHead();
        }

        void doWork(int time){
            working_thread = new thread([&, time]{
                sleep(time);
                cout << DataLinkFrameToString(**working_frame_p) << endl;
                delete(working_frame_p);
                working_frame_p = nullptr;
            });
        }
    };

    TEST(AutoPtr, FrameLinkedListThreading) {
        LinkedList<ESharedPtr<DataLinkFrame>> frame_queue;

        for(int i = 0; i < 3; i++){
            ESharedPtr<DataLinkFrame> data(new DataLinkFrame());
            data->msg_id = i;
            frame_queue.append(data);
        }

        EpopTest test1;
        test1.takeFromQueue(frame_queue);
        EpopTest test2;
        test2.takeFromQueue(frame_queue);

        /* try flush all! */
        for(int i = 0; i < 10; i++) {
            frame_queue.popHead();
        }
        test1.doWork(2);
        test2.doWork(1);

//        frame_0->msg_id = 10;
//        cout << DataLinkFrameToString(*frame_0) << endl;

        test1.working_thread->join();
        test2.working_thread->join();
        sleep(3);
    }
}

