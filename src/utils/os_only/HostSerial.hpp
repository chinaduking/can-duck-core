//
// Created by sdong on 2019/11/12.
//

#ifndef LIBFCN_SERIALFT232_HPP
#define LIBFCN_SERIALFT232_HPP

#include "utils/LLComDevice.hpp"
#include <vector>
#include <string>

#ifndef WIN32
#include <termios.h>
#else
#include <windows.h>
#include <atlstr.h>
#endif


namespace utils{
#ifndef B921600
    /*TODO: this only for mac!!*/
#define B921600 921600
#endif

    class HostSerial : public LLByteDevice {
    public:
        HostSerial(int id = 0, uint32_t baud = B921600, uint16_t read_timeout_ms = 0);
        ~HostSerial();
        int32_t read(uint8_t *data, uint32_t len) override ;
        int32_t write(const uint8_t *data, uint32_t len) override;

        bool isWriteBusy() override;

        int32_t sync();
        int32_t reinit();

        bool isOpen();

        inline uint32_t getBaud(){
            return baud;
        }

        static std::vector<std::string> listUSBDevice();

    private:
        bool is_open;
        int32_t serial_fd;
        uint32_t baud;
    };
}

#endif //LIBFCN_SERIALFT232_HPP
