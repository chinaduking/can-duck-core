//
// Created by sdong on 2020/10/12.
//

#ifndef LLCOMDEVICE_HPP
#define LLCOMDEVICE_HPP

#include <cstdint>

/* Low Level 底层设备接口：
 * 通用字节设备接口，需要实现读和写API */
class LLByteDevice{
public:
    LLByteDevice(uint32_t write_buf_size=128) :
        write_buf_size(write_buf_size){
        write_buf = new uint8_t[write_buf_size];
    }
    virtual ~LLByteDevice() {
        delete [] write_buf;
    }

    /* 读取
     *
     * 上位机：阻塞式地读取1字节
     *  在没有收到数据时阻塞，有任何数据都会立即从缓冲区弹出一字节，
     *  并返回。永远返回1
     *  线程阻塞时会自动休眠，避免高频轮询造成的CPU开销。
     *  只等待1字节，保证收取任何数据都会立即响应，保证实时性。
     *
     * 下位机：非阻塞地从缓冲区读取1个字节，未读取到则立即返回0。
     *  调用者有责任循环调用ByteIOFrameDevice::read，直到
     *  该byte_device->read返回0，以清空接收缓冲区
     * */
    virtual int32_t read(uint8_t *data, uint32_t len) = 0;


    /* 写入
     *
     * 上位机：非阻塞式地写入n字节。永远返回全部写入
     *
     * 下位机：非阻塞式地写入n字节，不一定能全部写入。
     * data将被拷贝到设备内部发送缓冲区。有两个原因：
     * 1.
     * */
    virtual int32_t write(const uint8_t *data, uint32_t len) = 0;

    /* 标志是否是阻塞式LL读取。 */
    bool is_blocking_recv{};

    uint8_t* write_buf;
    uint32_t write_buf_size;
};


/* Low Level 底层设备接口：
 * 通用字节设备接口，需要实现读和写API */
class LLCanBus{
public:
    struct Frame{
        uint64_t ExtId;
        uint8_t data[8];

    };

    LLCanBus() = default;
    virtual ~LLCanBus() = default;

    virtual int32_t read(Frame* frame) = 0;

    virtual int32_t write(Frame* frame) = 0;

    /* 标志是否是阻塞式LL读取。 */
    bool is_blocking_recv;
};

#endif //LLCOMDEVICE_HPP
