//
// Created by sdong on 2020/11/2.
//
#include "utils/os_only/HostSerial.hpp"
#include <cstring>
#include <cstdio>
#include <iostream>
#include <thread>
#include "utils/CppUtils.hpp"

using namespace utils;
using namespace std;
int main( int argc, char *argv[]){
    /* 0. ascii code
     * 1. hex  */
    uint8_t mode = 1;

    if(argc >= 2){
        if(strcmp(argv[1], "s") == 0){
            cout << "sterm: ascii char string mode " << endl;
            mode = 0;
        }
        else if(strcmp(argv[1], "h") == 0){
            cout << "sterm: hex mode " << endl;

            mode = 1;
        } else{
            cerr << "param is s or h" << endl;
            exit(-1);
        }
    }else{
        cout << "sterm: hex mode (defualt)" << endl;
    }

    int sid = 0;

    if(argc >= 3){
        sid = argv[2][0] - '0';

        if(sid < 0){
            cerr << "error: sid < 0" << endl;
            exit(-1);
        }

        if(sid > 9){
            cerr << "error: sid > 9" << endl;
            exit(-1);
        }
    }

    HostSerial serial(sid, B921600);

    thread kb_thread([&](){
        string kb_input_line;

        while(1){
            while (!utils::kbhit()) {
                utils::perciseSleep(0.05);
            }
            char c = getchar();
            serial.write((const uint8_t*)&c, 1);
        }
    });


    uint8_t line_wrap = 20;
    uint8_t line_wrap_cnt = 0;
    uint8_t data;

    while(serial.isOpen()){
        auto i = serial.read(&data, 1);

        if(mode == 0){
            putchar(data);
            line_wrap_cnt ++;
        }

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