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
        static std::vector<uint32_t> defaultBaudRates;

        HostSerial() = default;
        HostSerial(int id = 0,
                   uint32_t baud = B921600,
                   uint16_t read_timeout_ms = 0) : LLByteDevice(){
            open(id, baud, read_timeout_ms);
        }

        ~HostSerial();

        int open(std::string  port_name_,
                 uint32_t baud_ = B921600,
                 uint16_t read_timeout_ms = 0);

        int open(int id,
                 uint32_t baud = B921600,
                 uint16_t read_timeout_ms = 0);

        int close();

        std::vector<std::string> discoverPort();

        int32_t read (      uint8_t *data, uint32_t len) override ;
        int32_t write(const uint8_t *data, uint32_t len) override;
        int32_t sync();

        bool isOpen();
        inline bool isWriteBusy() override{ return !isOpen(); }
        inline uint32_t getBaud(){ return baud; }

    private:
        bool is_open  { false };
        uint32_t baud { 0     };

        int32_t  posix_serial_fd { 0 };
        std::string port_name { "-" };

#ifdef WIN32
        HANDLE hPort;
#endif
    };
}

#endif //LIBFCN_SERIALFT232_HPP
