//
// Created by sdong on 2019/11/11.
//

#ifndef LIBFCN_DATAVERIFY_HPP
#define LIBFCN_DATAVERIFY_HPP

#include <cstdint>

namespace libfcn_v2{

    uint16_t Crc16(uint8_t *pdata, uint16_t data_len);

    uint8_t  CheckSum1x1(uint8_t *pdata, uint16_t data_len);
    uint16_t CheckSum1x2(uint8_t *pdata, uint16_t data_len);
    uint32_t CheckSum1x4(uint8_t *pdata, uint16_t data_len);
    uint32_t CheckSum4x4(uint8_t *pdata, uint16_t data_len);

}

#endif //LIBFCN_DATAVERIFY_HPP
