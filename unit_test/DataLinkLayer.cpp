//
// Created by hlyl on 18-7-10.
//
#include "DataLinkLayer.hpp"
#include "FrameUtils.hpp"

#include "TestUtils.hpp"
#include "utils/PosixSerial.hpp"

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

        cout << DataLinkFrameToString(dst_frame) << endl;

        ASSERT_TRUE(DataLinkFrameCompare(src_frame, dst_frame));
    }


#if 1
    TEST(ByteStreamParserTest, io) {

        PosixSerial serial;
        ByteFrameIODevice frame_dev(&serial);

        char* src_test_buffer = (char*)"12345 12345";

        uint16_t src_buffer_len = strlen(src_test_buffer) + 1;

        auto src_frame = new DataLinkFrame();
//        ESharedPtr<DataLinkFrame> src_frame(new DataLinkFrame());

        src_frame->src_id = 0x03;
        src_frame->dest_id = 0x05;
        src_frame->op_code = 0x01;
        src_frame->msg_id = 0x07;
        src_frame->payload_len = src_buffer_len;
        memcpy(src_frame->payload, src_test_buffer, src_buffer_len);

        auto dest_frame = new DataLinkFrame();

        thread recv([&](){
            for(int i = 0; i < 1;){
                if(frame_dev.read(dest_frame)){
                    cout << DataLinkFrameToString(*dest_frame) << endl;
                    ASSERT_TRUE(DataLinkFrameCompare(*src_frame, *dest_frame));
                } else{
                    cout << "no more frame.." << endl;
                }
            }
        });

        for(int i = 0; i < 100; i ++){
            if(!serial.isOpen()){
                break;
            }
            frame_dev.write(src_frame);
            cout << "send..." << endl;
            sleep(1);
        }
        recv.join();
    }
#endif
}
