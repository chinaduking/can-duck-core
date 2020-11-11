//
// Created by hlyl on 18-7-10.
//
#include "utils/Tracer.hpp"
#include "libfcn/DataLinkLayer.hpp"
#include "TestUtils.hpp"
#include "utils/os_only/HostSerial.hpp"

using namespace libfcn_v2;
using namespace utils;

namespace frame_device_test{
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
        src_frame.payload_len = src_buffer_len;
        memcpy(src_frame.payload, src_test_buffer, src_buffer_len);

        FcnFrame dest_frame;

        thread recv([&](){
            for(int i = 0; i < 1;){
                if(frame_dev.popRxQueue(&dest_frame)){
                    cout << frame2log(dest_frame) << endl;
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
            sleep(1);
        }
        recv.join();
        send.join();
    }
}
