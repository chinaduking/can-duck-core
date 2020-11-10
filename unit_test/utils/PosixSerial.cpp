//
// Created by sdong on 2019/11/20.
//

#include "utils/os_only/PosixSerial.hpp"
#include "libfcn/TestUtils.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
using namespace utils;
using namespace std;


TEST(serial, recv){
    PosixSerial serial(0, B921600);

    if(serial.isOpen()){
        cout << "serial is open!" << endl;
    } else{
        return;
    }

    while(1){
        char data;
        auto i = serial.read((uint8_t*)&data, 1);
        cout << data;
    }
}

TEST(serial, loop){
    PosixSerial serial(0, B921600);

    if(serial.isOpen()){
        cout << "serial is open!" << endl;
    } else{
        return;
    }

    std::atomic<bool> stop_flag(false);

    thread t([&](){
        while(1){
            char data;
            auto i = serial.read((uint8_t*)&data, 1);
            cout << "read " << i << " byte :" << data << endl;
        }
    });

    for(auto i = 0; i < 10; i++){
        char* s = "hello world.";

        serial.write((uint8_t *)s, strlen(s) + 1);
        sleep(1);
    }

    t.join();
}


TEST(serial, dual){
    PosixSerial serial(0, B921600);
    PosixSerial serial1(1, B921600);


    if(serial.isOpen() && serial1.isOpen()){
        cout << "serial is open!" << endl;
    } else{
        return;
    }

    std::atomic<bool> stop_flag(false);

    thread t([&](){
        while(1){
            char data;
            auto i = serial1.read((uint8_t*)&data, 1);
            cout << data << endl;
//            cout << "read " << i << " byte :" << data << endl;
        }
    });

    for(auto i = 0; i < 1000; i++){
        char* s = "hello world.";

        serial.write((uint8_t *)s, strlen(s) + 1);
        perciseSleep(0.2);
    }

    t.join();
}