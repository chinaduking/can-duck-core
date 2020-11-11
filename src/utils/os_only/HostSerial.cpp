//
// Created by sdong on 2019/11/12.
//

#include "HostSerial.hpp"

#include "utils/Tracer.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <iostream>

#ifndef WIN32
    #include <unistd.h>
    #include <fcntl.h>
    #include <dirent.h>
#endif



using namespace utils;
using namespace std;

vector<uint32_t> HostSerial::defaultBaudRates = {
    2400,
    4800,
    9600,
    115200,
    256000,
    512000,
    921600,
    2000000,
    3000000
};

std::vector<std::string> HostSerial::discoverPort() {
#ifndef WIN32
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
#else  //WIN32
    vector<string> usb_serial;
    //TODO: discover win serial
    return usb_serial;
#endif  //WIN32
}


int HostSerial::open(std::string port_name_,
                     uint32_t baud_,
                     uint16_t read_timeout_ms) {
    if(is_open){
        LOGE("port %s is already opened!", this->port_name.c_str());
        return -3;
    }
    is_open = false;

#ifndef WIN32
    posix_serial_fd = ::open(port_name_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if(posix_serial_fd < 0) {
        LOGE("open file failed");
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if(tcgetattr(posix_serial_fd, &tty) != 0) {
        LOGE("tcgetattr failed");
        return -2;
    }

    cfsetispeed(&tty, baud_);
    cfsetospeed(&tty, baud_);

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

    if(tcsetattr(posix_serial_fd, TCSANOW, &tty) != 0) {
        LOGE("tcsetattr failed");
        return -2;
    }

    is_open = true;

    this->port_name = port_name_;
    this->baud = baud_;

    return 0;

#else  //WIN32
    DCB dcb;
    CString PortSpecifier = port_name_.c_str();

    HANDLE hPort = CreateFile(
            PortSpecifier,
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
    );

    if (!GetCommState(hPort,&dcb)){
        return -1;
    }

    dcb.BaudRate = baud_; //9600 Baud
    dcb.ByteSize = 8; //8 data bits
    dcb.Parity   = NOPARITY; //no parity
    dcb.StopBits = ONESTOPBIT; //1 stop

    if (!SetCommState(hPort,&dcb)){
        return -2;
    }

    is_open = true;

    this->port_name = port_name_;
    this->baud = baud_;

    return 0;
#endif  //WIN32
}

int HostSerial::open(int id, uint32_t baud, uint16_t read_timeout_ms) {
    is_open = false;

    auto serial_devices = discoverPort();

    if(serial_devices.size() == 0){
        LOGE("no usb serial found!!");
        return -1;
    }

    if(id >= serial_devices.size()){
        LOGE("too large serial num. only %d devices are found!",
             serial_devices.size());
        return -2;
    }

    LOGD("opening %s", serial_devices[id].c_str());
    int res = open(serial_devices[id], baud, read_timeout_ms);
    LOGD("opened %s", port_name.c_str());

    return res;
}


int HostSerial::close() {
#ifndef WIN32
    ::close(posix_serial_fd);
#else  //WIN32
    CloseHandle(hPort);
#endif  //WIN32

    return 0;
}

bool HostSerial::isOpen() {
    return is_open;
}

HostSerial::~HostSerial() {
    LOGD("close serial..");
    if(!is_open){ return; }
    close();
}


int32_t HostSerial::read(uint8_t *data, uint32_t len) {
    if(!is_open){ return 0; }
#ifndef WIN32
    int res = (int32_t)::read(posix_serial_fd, data, len);
    return res;
#else  //WIN32
    DWORD dwBytesTransferred;
    DWORD dwCommModemStatus;

    SetCommMask (hPort, EV_RXCHAR | EV_ERR); //receive character event
    WaitCommEvent(hPort, &dwCommModemStatus, 0); //wait for character

    if (dwCommModemStatus & EV_RXCHAR)
        ReadFile (hPort, data, len, &dwBytesTransferred, 0); //read
    else if (dwCommModemStatus & EV_ERR)
        return 0;
    return dwBytesTransferred;
#endif  //WIN32
}


int32_t HostSerial::write(const uint8_t *data, uint32_t len) {
    if(!is_open){ return 0; }
#ifndef WIN32
    return (int32_t)::write(posix_serial_fd, data, len);
#else  //WIN32
    DWORD byteswritten;
    CString data_str = data; //TODO: build from *data
    WriteFile(hPort,data_str,len,&byteswritten,NULL);
    return byteswritten;
#endif  //WIN32
}

int32_t HostSerial::sync() {
#ifndef WIN32
    return ::fsync(posix_serial_fd);
#else  //WIN32
    return 0;
#endif  //WIN32
}