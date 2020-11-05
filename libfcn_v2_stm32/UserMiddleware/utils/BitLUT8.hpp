//
// Created by sdong on 2019/11/23.
//

#ifndef LIBFCN_BITLUT8_HPP
#define LIBFCN_BITLUT8_HPP

#include <cstdint>
namespace utils{
    class BitLUT8{
    public:
        BitLUT8() = default;
        ~BitLUT8() = default;

        inline void add(uint8_t x){
            bit_map[(x >> 5)] |= (1 << (x & 0x3F));
        }

        inline bool has(uint8_t x){
            return (bit_map[(x >> 5)] & (1 << (x & 0x3F))) != 0;
        }

        inline void remove(uint8_t x){
            bit_map[(x >> 5)] &= ~(1 << (x & 0x3F));
        }

    private:
        uint32_t bit_map[4]{0,0,0,0};
    };

}

#endif //LIBFCN_BITLUT8_HPP
