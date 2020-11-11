//
// Created by sdong on 2019/11/12.
//

#include "HostSerial.hpp"

#include "utils/Tracer.hpp"

#include <stdint.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <dirent.h>
#include <iostream>

using namespace utils;
using namespace std;

HostSerial::HostSerial(int id, uint32_t baud,
                       uint16_t read_timeout_ms ): LLByteDevice(),
                                                     baud(baud){
    /*TODO: use FTDI device ID to open serial (FTDI-D2XX Driver)
     * AR: @jin.wang*/
    is_open = false;


    auto serial_devices = listUSBDevice();

    if(serial_devices.size() == 0){
        LOGE("no usb serial found!!");
        return ;
    }

    if(id >= serial_devices.size()){
        LOGE("too large serial num. only %d devices are found!",
             serial_devices.size());

        return ;
    }

    LOGD("opening %s", serial_devices[id].c_str());

    serial_fd = open(serial_devices[id].c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if(serial_fd < 0) {
        LOGE("open file failed");
        return;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if(tcgetattr(serial_fd, &tty) != 0) {
        return;
    }

    cfsetispeed(&tty, baud);
    cfsetospeed(&tty, baud);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays

    /*refer to: http://www.unixwiz.net/techtips/termios-vmin-vtime.html*/
    /*blocking forever */
    if(read_timeout_ms == 0){
        tty.c_cc[VMIN]  = 1;            // read is block
        tty.c_cc[VTIME] = 1;            // wait forever
    } else{

        tty.c_cc[VMIN]  = 0;            // read doesn't block

        if(read_timeout_ms > 20000){
            read_timeout_ms = 20000;
        }
        if(read_timeout_ms <= 100){
            read_timeout_ms = 101;
        }

        tty.c_cc[VTIME] = read_timeout_ms / 100;            //0.n seconds read timeout
    }
    is_blocking_recv = true;


    tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);     // shut off xon/xoff ctrl; Don't translate CR to newline

    tty.c_cflag |= (CLOCAL | CREAD);                    // ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);                  // shut off parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if(tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        return;
    }

    is_open = true;

    LOGD("opened %s", serial_devices[id].c_str());
}

bool HostSerial::isOpen() {
    return is_open;
}

HostSerial::~HostSerial()
{
    LOGD("close serial..");

    if(!is_open){ return; }
    close(serial_fd);
}


int32_t HostSerial::read(uint8_t *data, uint32_t len) {
    if(!is_open){ return 0; }

    int res = (int32_t)::read(serial_fd, data, len);

    return res;
}

bool HostSerial::isWriteBusy() {
    return false;
}

int32_t HostSerial::write(const uint8_t *data, uint32_t len) {
    if(!is_open){ return 0; }
    return (int32_t)::write(serial_fd, data, len);
}

int32_t HostSerial::sync() {
    return fsync(serial_fd);
}

int32_t HostSerial::reinit() {

    return 0;
}

std::vector<std::string> HostSerial::listUSBDevice(){
    vector<string> usb_serial;

    char* pattern_unix[] = {
            (char*)"ttyUSB",   /*Ubuntu下的串口设备*/
            (char*)"cu.usbserial-", /*Mac下的串口设备*/
            (char*)"ttyS"  /*Android下的串口设备*/
    };


    DIR *dir;
    struct dirent *ent;

    string dev_dir = "/dev";
    if ((dir = opendir (dev_dir.c_str())) != NULL) {
        /* 列出路径下所有文件 */
        while ((ent = readdir (dir)) != NULL) {
//            printf ("%s\n", ent->d_name);
            for(auto pattern : pattern_unix){
                if(strncmp(pattern, ent->d_name, strlen(pattern)) == 0){
                    LOGD("\nFound USB Serial Device: %s\n", ent->d_name);
                    usb_serial.push_back(dev_dir + "/" + string(ent->d_name));
                }
            }

        }
        closedir (dir);
    } else {
        /* could not open directory */
        LOGF("could not open directory /dev");
        throw runtime_error("could not open directory /dev");
    }

    return usb_serial;
}

