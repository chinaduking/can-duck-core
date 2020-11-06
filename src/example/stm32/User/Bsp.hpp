#ifndef BSPINIT_H
#define BSPINIT_H



/*
 * 声明设备实例。应给设备起一个和其板上功能相对应的名字
 * */
#include "hal_cpp_wrapper/Usart.hpp"
extern mcu_driver::Usart hostSerial;

#include "hal_cpp_wrapper/Can.hpp"
extern mcu_driver::Can can1;

extern mcu_driver::Can can2;

#include "hal_cpp_wrapper/Gpio.hpp"
extern mcu_driver::Gpio led[3];
/* 计算核心频率
 * */
#define CORE_FREQ_KHZ 168000



/*
 * 板级支持包运行时初始化
 * */
void BspInit();


#endif //BSPINIT_H
