<img src="docs/img/libfcn.png" width="200" />  

# LIBFCN 2.0 - Flexible Controller Network<br>灵活的控制器网络 

## 特性  
1. **适用于微控制器的实时消息中间件**：赋能消费电子、机器人、工业控制领域，产品级、快速、可靠的原型开发。<br>
2. **兼容性**：<br>
    跨总线：支持共享内存、CAN、UART、RS485、以太网等，可无缝、无感地衔接不同种类通信方式。<br>
    跨平台：一套代码可支持STM32裸机，Windows，Linux，MacOSX
3. **发布者-订阅者 Publisher-Subscriber：** 分布式实时通信
4. **参数服务器 Parameter Server：** 可靠的同步/异步参数读写服务
5. **远程过程调用 RPC：** 可靠的同步/异步请求发送和应答响应
6. **远程固件升级 OTA Update** 可对任意一个或多个节点进行可靠的固件下载
7. **接口定义语言 IDL：** 消息API自动生成脚本，快速适配新需求
8. **超轻量：** 可直接运行于无RTOS的STM32，占用不到10kROM。全部任务靠事件循环驱动，无需中断。消息收发及调度额外开销几乎为零，效率接近直接面向寄存器开发。
9. **丰富的调试工具：** 
    - 总线调试器FCNProbe: 硬件+上位机，高帧率数据记录、回放。上位机Windows，Linux，MacOSX全平台兼容。
    - FCNVis: 数据可视化上位机，Windows，Linux，MacOSX全平台兼容。
    - Tracer: MCU/上位机通用的轻量C++调试信息打印库，可自定义过滤等级、输出目标，在终端中彩色显示。





详细文档见[这里](docs/doxygen/html/index.html) 