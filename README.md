

# CAN-Duck:- A Distributed MICROcontroller Communication stacK based on CAN -- 基于CAN总线的分布式微控制器通信协议栈

<img src="docs/img/logo-mid.png" width = "300" align=left />

## 0. 什么是CAN-Duck？

0.1 微控制器通信典型场景

机器人、消费电子、工业控制领域中，经常需要将系统功能分散在多个MCU中实现，以满足电气连接的约束，或提升系统的模块化程度。以下是一个典型的机器人产品MCU连接图：

<img src="docs/img/demo-sys.png" width = "600" align=center />

## 1. 特性

- **适用于微控制器的实时消息中间件 ：** 赋能消费电子、机器人、工控领域，产品级、快速、可靠的原型开发。

- **兼容性：**
    - 跨总线：支持共享内存、CAN、UART、RS485、以太网等，可无缝、无感地衔接不同种类通信方式。
    - 跨平台：一套代码可支持STM32裸机，Windows，Linux，MacOSX
    
- **分布式：** 网络拓扑为多主（Multi-Master）软总线，无需中心节点，便于更改网络结构。
- **发布者-订阅者 Publisher-Subscriber：** 多对多实时消息收发。
- **参数服务器 Parameter Server：** 可靠的同步/异步参数读写服务。
- **远程过程调用 RPC：** 可靠的同步/异步请求发送和应答响应。
- **远程固件升级 OTA Update：** 可对任意一个或多个节点进行可靠的固件下载。
- **接口定义语言 IDL：** 面向用户的消息API由脚本自动生成，用户只需定义消息名称和类型。可快速适配新需求。
- **节点映射 Context Mapping：** 同一个MCU上可以有多个通信节点，每个节点均可像在一个独立的MCU
上一样工作：每个节点拥有独立的地址，可通过本地或网络上任一其它节点直接访问。这使得硬件架构的变化变得简单：比如，不论某传感器/执行器位于哪个MCU
，均可以以相同的方式访问它。而同一MCU上多节点之间采用共享内存通信，和使用全局变量直接传递数据效率相同，在需要无延迟的高性能控制场景也适用。
- **超轻量：** 可直接运行于无RTOS的STM32，占用不到10kROM，无需中断请求。消息收发及调度额外开销几乎为零，效率接近直接面向寄存器开发。
- **丰富的调试工具：** 
    - **FCNProbe：** 总线调试器。硬件+上位机，将高帧率的数据包和串口Log进行汇总、转发、记录、回放。上位机Windows，Linux
    ，MacOSX
    全平台兼容。
    - **FCNVis:：** 实时数据可视化上位机。Windows，Linux，MacOSX全平台兼容。
    - **Tracer：** MCU/上位机通用的轻量C
    ++调试信息打印库，可自定义过滤等级、输出目标，在终端中彩色显示。内置缓冲区，因此打印API的调用为并发、非阻塞式的，且在MCU
    中已配置为使用DMA进行串口输出。因此即便在Debug版本输出较多的调试信息，程序行为和Release版本也几乎无差异。

--------------

## 2. 克隆仓库并运行第一个测试
### 2.1 前置需求

- Windows
  - Windows 10
  - CLion 2020 (可选)
  - STM32CubeIDE
  - Visual Studio 2019 Community  
  - Git / vckpg / CMake / doxygen(1.8.15+) / Python3
  - GTest (从vckpg安装)
  - FT232 VCom Driver  
  <br>
*提示：您可能需要将Windows的系统编码格式改为UTF-8以消除编译错误*
  <br>
- Linux & MacOSX
  - CLion 2020  (可选)
  - STM32CubeIDE
  - Git / vckpg / CMake / doxygen(1.8.15+) / Python3
  - GTest

### 2.2 通过命令行编译运行测试用例

### 2.2 在CLion IDE中运行测试用例


--------------
## 3. 使用入门
### 3.1 启动节点并添加网络设备

### 3.2 发布者-订阅者
**定义消息**

**发布消息**

**订阅消息**

### 3.3 参数服务器
**定义消息**

**服务器**

**客户端**


```c

/**
*                      (Host/Monitor)
*                          |
*                  [ Serial/Ethernet ]
*                          |
*  (subnet A) ----{[CAN1]-[DCS]-[CAN2]}------(subnet B)
*
* */


//fast msg ext (29 bit CAN ID)
//is seg   1 bit        [ =0]   
//is msg   1 bit        [ =1]
//node id  6 bit        [1-63 & 0]
//tx/rx    1 bit        [0/1]
//msg  id  3 bit        [0-7] 
//empty n  1 bit        [0/1]
//data[0]                           
//data[1]
//data 2-9


//sevice (29 bit CAN ID)
//is seg  1  bit        [ =0]     
//is msg  1  bit        [ =0]
//src id  6  bit        [ 1-63 & 0(anou) ]
//dest id 6  bit        [ 1-63 & 0(anou) ]
//op code 5  bit        [ 0-31   ]   
//srv_id  10 bit        [ 0-1023 ]


//fast msg std (11 bit CAN ID)  (TODO..)
//is seg                [0/1]  (if data follow uart's protocol)
//node id  6 bit        [1-63 & 0(anou)]
//tx/rx    1            [0/1]               
//msg  id  3 bit    tx: [0-7]  + (offset=8)  
//data 0-7

//segmented msg (29 bit CAN ID)   (TODO..)
//is seg     1 bit      [ =1]   
//src id     6 bit      [ 1-63 & 0(anou) ]
//dest id    6 bit      [ 1-63 & 0(anou) ]
//trans id   6 bit                       
//s/t/e      2 bit      [0/1/2]
//data[0]    : pack_n/pack_c(down count)
//data[1-8]  : merged:{srv_id/srv_op+srv_code, data}


```


详细文档见[这里](docs/doxygen/html/index.html) 