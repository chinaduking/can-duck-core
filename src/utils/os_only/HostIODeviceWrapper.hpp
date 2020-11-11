//
// Created by sdong on 2019/11/17.
//

#ifndef LIBFCN_STDIO_HPP
#define LIBFCN_STDIO_HPP

#include "utils/LLComDevice.hpp"

#include <string>
namespace utils{
    class StdoutIODviceWrapper : public LLByteDevice{
    public:
        StdoutIODviceWrapper() = default;
        ~StdoutIODviceWrapper() override = default;
        int32_t read(uint8_t *data, uint32_t len) override ;
        int32_t write(const uint8_t *data, uint32_t len) override ;
        inline bool isWriteBusy() override{
            return false;
        }
    };

    class FileIODviceWrapper : public LLByteDevice{
    public:
        FileIODviceWrapper(std::string path);
        ~FileIODviceWrapper() override = default;
        int32_t read(uint8_t *data, uint32_t len) override ;
        int32_t write(const uint8_t *data, uint32_t len) override ;

        inline bool isWriteBusy() override{
            return false;
        }

    private:
        std::string path;
        FILE* fd;
    };

}

#endif //LIBFCN_STDIO_HPP
