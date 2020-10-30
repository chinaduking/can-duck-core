//
// Created by sdong on 2019/11/11.
//

#ifndef LIBFCN_FRAMEUTILS_HPP
#define LIBFCN_FRAMEUTILS_HPP

#include <string>
#include "DataLinkLayer.hpp"

namespace libfcn_v2{



    inline std::string DataLinkFrameToString(DataLinkFrame& frame){
        static const int BUFFER_RESERVE = 120;

        char buffer[DATALINK_MTU * 4 + BUFFER_RESERVE];
        
        if(frame.payload_len > DATALINK_MTU){
            return std::string("::: DataLinkFrame  > DATALINK_MTU\n");
        }

        sprintf(buffer, "::: DataLinkFrame \n"
                        "\tdest  id = 0x%.2X\n"
                        "\tsrc   id = 0x%.2X\n"
                        "\top code  = 0x%.2X\n"
                        "\tmsg   id = 0x%.2X\n"
                        "\tpayload[%.3d] = \n"
                        "\t\t(hex) ",
                frame.dest_id  & 0xff,
                frame.src_id   & 0xff,
                frame.op_code  & 0xff,
                frame.msg_id   & 0xff,
                frame.payload_len);

        static int info_offset = 0;

        if(info_offset == 0){
            info_offset = strlen(buffer);
        }

        if(info_offset > BUFFER_RESERVE){
            return std::string("::: info_offset > BUFFER_RESERVE\n");
        }

        for(int i = 0; i < frame.payload_len; i ++){
            sprintf(&buffer[info_offset + i * 3], "%.2X ", frame.payload[i] & 0xff);
        }

        sprintf(&buffer[info_offset + frame.payload_len * 3], "\n\t\t(str) %s\n", frame.payload);

        return std::string(buffer);
    }

    inline std::string DataLinkFrameToStringCompact(DataLinkFrame& frame){
        static const int BUFFER_RESERVE = 120;

        char buffer[DATALINK_MTU * 4 + BUFFER_RESERVE];

        if(frame.payload_len > DATALINK_MTU){
            return std::string("::: DataLinkFrame  > DATALINK_MTU\n");
        }

        sprintf(buffer, "\n\t::: DataLinkFrame "
                        "D[0x%.2X] "
                        "S[0x%.2X] "
                        "O[0x%.2X] "
                        "I[0x%.2X] "
                        " \n\t\t D[%.3d] = "
                        "",
                frame.dest_id  & 0xff,
                frame.src_id   & 0xff,
                frame.op_code  & 0xff,
                frame.msg_id   & 0xff,
                frame.payload_len);

        static int info_offset = 0;

        if(info_offset == 0){
            info_offset = strlen(buffer);
        }

        if(info_offset > BUFFER_RESERVE){
            return std::string("::: info_offset > BUFFER_RESERVE\n");
        }

        for(int i = 0; i < frame.payload_len; i ++){
            sprintf(&buffer[info_offset + i * 3], "%.2X ", frame.payload[i] & 0xff);
        }

        return std::string(buffer);
    }

    inline bool DataLinkFrameCompare(DataLinkFrame& frame1, DataLinkFrame& frame2){
        if(frame1.payload_len != frame2.payload_len){
            return false;
        }

        if(frame1.msg_id != frame2.msg_id){
            return false;
        }

        if(frame1.src_id != frame2.src_id){
            return false;
        }

        if(frame1.dest_id != frame2.dest_id){
            return false;
        }

        if(frame1.op_code != frame2.op_code){
            return false;
        }

        return memcmp(frame1.payload, frame2.payload, frame1.payload_len) == 0;
    }

    inline bool DataLinkFramePayloadCompare(DataLinkFrame& frame1, DataLinkFrame& frame2){
        if(frame1.payload_len != frame2.payload_len){
            return false;
        }

        return memcmp(frame1.payload, frame2.payload, frame1.payload_len) == 0;
    }
}

#endif //LIBFCN_FRAMEUTILS_HPP
