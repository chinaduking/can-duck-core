//
// Created by sdong on 2020/11/2.
//
#include "utils/PosixSerial.hpp"

using namespace utils;

int main(){
    PosixSerial serial(0, B921600);
    uint8_t data;
    /* 0. ascii code
     * 1. hex  */
    uint8_t mode = 1;


    uint8_t line_wrap = 20;
    uint8_t line_wrap_cnt = 0;

    while(serial.isOpen()){
        auto i = serial.read(&data, 1);

        if(mode == 1){
            printf("%.2X ", data);
            if(line_wrap_cnt > line_wrap){
                printf("\n\r ", data);
                line_wrap_cnt = 0;
            }
            line_wrap_cnt ++;
        }


        fflush(stdout);
    }

    return 0;
}
