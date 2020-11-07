//
// Created by hlyl on 18-7-10.
//
#include "utils/Tracer.hpp"
#include "libfcn/DataLinkLayer.hpp"
#include "TestUtils.hpp"
#include "utils/os_only/PosixSerial.hpp"

using namespace libfcn_v2;
using namespace utils;

namespace frame_device_test{

    TEST(ByteStreamParserTest, frame2Buffer){
        char* src_test_buffer = (char*)"hello world";

        uint16_t src_buffer_len = strlen(src_test_buffer) + 1;

        DataLinkFrame src_frame, dst_frame;

        src_frame.src_id = 0x03;
        src_frame.dest_id = 0x05;
        src_frame.op_code = 0x01;
        src_frame.msg_id = 0x07;
        src_frame.payload_len = src_buffer_len;
        memcpy(src_frame.payload, src_test_buffer, src_buffer_len);

        uint8_t buffer[200] = {0};
        uint16_t len = 0;

        len = frame2Buffer(&src_frame, buffer);
        buffer2Frame(&dst_frame, buffer+1, len-1);

        cout << frame2log(dst_frame) << endl;

        ASSERT_TRUE(DataLinkFrameCompare(src_frame, dst_frame));
    }


    TEST(ByteStreamParserTest, assign){
        char* src_test_buffer = (char*)"hello world";

        uint16_t src_buffer_len = strlen(src_test_buffer) + 1;

        DataLinkFrame src_frame, dst_frame;

        src_frame.src_id = 0x03;
        src_frame.dest_id = 0x05;
        src_frame.op_code = 0x01;
        src_frame.msg_id = 0x07;
        src_frame.payload_len = 4;
        memcpy(src_frame.payload, src_test_buffer, src_buffer_len);

        dst_frame = src_frame;

        cout << frame2log(dst_frame) << endl;

        DataLinkFrame f_array[1];
        cout << sizeof(f_array) << endl;

        ASSERT_TRUE(DataLinkFrameCompare(src_frame, dst_frame));
    }




    TEST(ByteStreamParserTest, io) {

        PosixSerial serial(1);
        ByteFrameIODevice frame_dev(&serial);

        char* src_test_buffer = (char*)"12345";

        uint16_t src_buffer_len = strlen(src_test_buffer) + 1;

        DataLinkFrame src_frame;
//        ESharedPtr<DataLinkFrame> src_frame(new DataLinkFrame());

        src_frame.src_id  = 0x03;
        src_frame.dest_id = 0x05;
        src_frame.op_code = 0x01;
        src_frame.msg_id  = 0x07;
        src_frame.payload_len = src_buffer_len;
        memcpy(src_frame.payload, src_test_buffer, src_buffer_len);

        DataLinkFrame dest_frame;

        thread recv([&](){
            for(int i = 0; i < 1;){
                if(frame_dev.read(&dest_frame)){
                    cout << frame2log(dest_frame) << endl;
                    ASSERT_TRUE(DataLinkFrameCompare(src_frame, dest_frame));
                } else{
                    cout << "no more frame.." << endl;
                }
            }
        });

        thread sendpoll([&](){
            for(int i = 0; i < 1;){

                while(frame_dev.writePoll());
                perciseSleep(0.1);
            }
        });

        for(int i = 0; i < 100; ){
            if(!serial.isOpen()){
                break;
            }
            frame_dev.write(&src_frame);
            frame_dev.write(&src_frame);
            cout << "send x2..." << endl;
            sleep(1);
        }
        recv.join();
        sendpoll.join();
    }
}
