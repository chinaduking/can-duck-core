//
// Created by hlyl on 18-7-10.
//
#include "DataLinkLayer.hpp"
#include "TestUtils.hpp"
#include "Tracer.hpp"
#include "HostSerial.hpp"

using namespace can_duck;
using namespace emlib;

namespace frame_device_test{
    TEST(DataLinkLayer, fram2log) {

        char* src_test_buffer = (char*)"12345 12345";

        uint16_t src_buffer_len = strlen(src_test_buffer) + 1;

        FcnFrame src_frame;
//        ESharedPtr<DataLinkFrame> src_frame(new DataLinkFrame());

        src_frame.src_id  = 0x03;
        src_frame.dest_id = 0x05;
        src_frame.op_code = 0x01;
        src_frame.msg_id  = 0x07;
        src_frame.setPayloadLen(64);
        memcpy(src_frame.payload, src_test_buffer, src_buffer_len);

        cout << frame2stdstr(src_frame) << endl;


    }

    TEST(DataLinkLayer, SerialFrameDev) {

        HostSerial serial(0);
        ByteFrameIODevice frame_dev(&serial);

        char* src_test_buffer = (char*)"12345 12345";

        uint16_t src_buffer_len = strlen(src_test_buffer) + 1;

        FcnFrame src_frame;
//        ESharedPtr<DataLinkFrame> src_frame(new DataLinkFrame());

        src_frame.src_id  = 0x03;
        src_frame.dest_id = 0x05;
        src_frame.op_code = 0x01;
        src_frame.msg_id  = 0x07;
        src_frame.setPayloadLen(src_buffer_len);
        memcpy(src_frame.payload, src_test_buffer, src_buffer_len);

        cout << frame2stdstr(src_frame) << endl;

        FcnFrame dest_frame;

        thread recv([&](){
            for(int i = 0; i < 1;){
                if(frame_dev.popRxQueue(&dest_frame)){
                    cout << frame2stdstr(dest_frame) << endl;
                    ASSERT_TRUE(DataLinkFrameCompare(src_frame, dest_frame));
                } else{
                    cout << "no more frame.." << endl;
                }
            }
        });

        thread send([&](){
            for(int i = 0; i < 1;){
                frame_dev.sendPolling();
            }
        });

        for(int i = 0; i < 100; ){
            if(!serial.isOpen()){
                break;
            }
            frame_dev.pushTxQueue(&src_frame);
            frame_dev.pushTxQueue(&src_frame);
            cout << "send x2..." << endl;
            perciseSleep(1);
        }
        recv.join();
        send.join();
    }
}
