

# CAN-Duck: A Distributed MICROcontroller Communication stacK based on CAN 

**基于CAN总线的分布式微控制器通信协议栈**

<img src="docs/img/logo-mid.png" width = "300" align=center />


- [CAN-Duck: A Distributed MICROcontroller Communication stacK based on CAN](#can-duck-a-distributed-microcontroller-communication-stack-based-on-can)
  - [1. 什么是CAN-Duck？](#1-什么是can-duck)
    - [1.1 微控制器通信典型场景](#11-微控制器通信典型场景)
    - [1.2 为什么需要通信协议栈？](#12-为什么需要通信协议栈)
    - [1.3 CAN-Duck vs CAN-Open](#13-can-duck-vs-can-open)
  - [2. 克隆仓库并运行第一个测试](#2-克隆仓库并运行第一个测试)
    - [2.1 前置需求](#21-前置需求)
    - [2.2 通过命令行编译运行测试用例](#22-通过命令行编译运行测试用例)
    - [2.2 在CLion IDE中运行测试用例](#22-在clion-ide中运行测试用例)
  - [3. 使用入门](#3-使用入门)
    - [3.1 启动节点并添加网络设备](#31-启动节点并添加网络设备)
    - [3.2 发布者-订阅者](#32-发布者-订阅者)
    - [3.3 参数服务器](#33-参数服务器)


## 1. 什么是CAN-Duck？

### 1.1 微控制器通信典型场景
机器人、消费电子、工业控制领域中，经常需要将系统功能分散在多个MCU中实现，以满足电气连接的约束，或提升系统的模块化程度。以下是一个典型的机器人产品MCU连接图：

<img src="docs/img/demo-sys.png" width = "500" align=center />

一般将要求硬实时的运动控制算法运行在一个MCU中，采用bare metal架构（无操作系统），并用CAN总线等和执行器进行通信；执行器的MCU接收运动控制指令，并运行伺服驱动程序（如FOC、三闭环）。而对算力需求更高的人机界面、感知决策算法运行在通用处理器中，采用Linux/Windows等通用操作系统。这样也能节省采购RTOS的成本并对系统进行快速验证。
    
另一方面，对于某些对成本约束极严的产品，常会将高度优化后的伺服驱动程序和实时运动控制、甚至传感器采集程序都全部集中在一块MCU中，这一MCU此时被称为**域控制器（DCU）**，此时系统架构如下：

<img src="docs/img/demo-sys-dcu.png" width = "500" align=center />

一般来讲，采用分布式架构还是DCU架构，取决于产品开发的阶段。早期验证时，为了加快迭代速度，一般采用分布式架构，直接集成硬件模块；后期产品功能明确后，为了降低成本，可能会换为DCS架构。

### 1.2 为什么需要通信协议栈？

CAN、串口的底层原理都非常简单，但面对复杂的业务需求，如：需要在有限的MTU约束下传输多种不同功能的数据包；对远程MCU的内部数据进行可靠的访问；对不同功能、不同总线上的MCU采用统一的API进行控制等，就不能直接利用通信硬件的底层数据直接满足这些需求，而必须对数据传输的链路层、协议层、表示层进行抽象和封装。
CAN总线常采用的协议栈是CANOpen，这一协议栈已在工业领域广泛应用；串口一般采用ModBus协议或AT指令等。另外还有UAVCAN等业余爱好者开发的协议。

### 1.3 CAN-Duck vs CAN-Open

既然已经有了CANOpen，我们为什么仍要开发CAN-Duck？CAN-Duck具有以下几方面的优势。

- **开发语言：** 

    CAN-Duck一开始就采用**C++11**开发，结合**接口描述语言**IDL和C++模板带来的**编译期类型检查**能力，使得对象字典可被方便和安全地访问。相比之下CANOpen的几个开源协议栈（CANOpenNode、Lely CAN）等较为保守地采用C语言作为原始的内核，在使用上较为复杂。且得益于C++标准库的**跨平台**能力，CAN-Duck协议栈可一套代码同时运行在MCU、Linux、Windows、Mac上。

- **实时数据：** 

    CANOpen的应用场景是具有大量状态量的工业系统，因此采用字典映射的方式压缩Index以有效传输实时数据，但这也带来了额外复杂度。CAN-Duck的数据包针对现代机器人和消费电子等领域进行优化，以最低的学习验证成本实现了实时数据传输。且CAN-Duck充分利用了CAN2.0的29位扩展ID，使得单包实时数据的MTU支持到了**10字节**，降低了分段传输的额外开销。

- **Pub/Sub模式的支持：** 

    CAN-Duck直接提供封装好的**发布者-订阅者API**，接口简洁明了，无需为了传输实时数据而重新学习CANOpen的PDO格式。

- **C/S模式的支持：** 

    对于可靠地访问远程节点的内部状态，CANOpen仅在协议层定义了SDO数据格式，至于上层API则需要用户自行实现。因C/S模式需要比实时数据更复杂的数据解析，且要实现异步API，因此比Pub/Sub更为复杂。CAN-Duck不仅支持C/S模式，且实现了一个基于事件循环的客户端请求管理器、并将发起请求的接口和对象字典进行了**很好的融合**，大大简化了客户端的编程工作。上述事件循环可在无操作系统的MCU中直接运行。


- **多总线统一接口：**

    CAN-Duck虽然基于CAN总线，但也实现了一个基于字节设备的较为完善的**CAN数据包模拟**，因此可以使得串口、UDP网口等无缝转发CAN的数据包。得益于此，CAN-Duck可以很好地在小至Arduino，大至带CAN-Ethernet网关的复杂网络中工作。你甚至不需要CAN转换器即可使用完整的CAN-Duck协议栈。



- **单MCU虚拟多节点的能力：**
    这是CAN-Duck比CANOpen更灵活的另一创新点：在传统的CAN通信中，如果分布式架构中某一MCU的功能要集成在DCU中，则上位机、DCU中所有涉及该MCU通信相关的代码均需要做出更改，这减慢了研发的进度。在CAN-Duck中，一个MCU可以虚拟多个节点，每个节点均具有CAN-Duck协议栈的**完整功能**，可对同一MCU上的其它虚拟节点或另一MCU上的远程节点进行通信，而API是完全统一的。对于同一MCU上的虚拟节点，实时数据的传输采用了共享内存的方式，效率和直接使用全局变量传递数据是相同的。

- **集成文件传输协议：**  
    CAN-Duck实现了一个简单的文件传输协议，可用于OTA、记录读取等。该文件传输协议的服务端和客户端也均可在无OS的MCU中直接运行。


- **丰富的调试工具：**   
    CAN-Duck已经集成了多种方便的调试工具。
	- **DuckProbe：** 总线调试器。硬件+上位机，将高帧率的数据包和串口Log进行汇总、转发、记录、回放。上位机Windows，Linux ，MacOSX全平台兼容。
	- **DuckPlot:：** 实时数据可视化上位机。Windows，Linux，MacOSX全平台兼容。
	- **Tracer：** MCU/上位机通用的轻量C++调试信息打印库，可自定义过滤等级、输出目标，在终端中彩色显示。内置缓冲区，因此打印API的调用为并发、非阻塞式的，且在MCU中已配置为使用DMA进行串口输出。因此即便在Debug版本输出较多的调试信息，程序行为和Release版本也几乎无差异。

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