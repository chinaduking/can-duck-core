//
// Created by sdong on 2019/11/11.
//
#include "DataVerify.hpp"

using namespace libfcn_v2;

/*CRC-16 impl:
 * http://www.darkridge.com/~jpr5/mirror/alg/node191.html
 *
 * x^16 + x^5 + x^2 + 1
*/
const uint16_t crc_16_table[] =
        {
                0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
                0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
        };

uint16_t libfcn_v2::Crc16(uint8_t* pdata, uint16_t data_len)
{
    uint16_t wCRC = 0xFFFF;
    uint16_t i;
    uint8_t  chChar;
    for (i = 0; i < data_len; i++) {
        chChar = *pdata++;
        wCRC = crc_16_table[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
        wCRC = crc_16_table[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
    }
    return wCRC;
}


uint8_t libfcn_v2::CheckSum1x1(uint8_t *pdata, uint16_t data_len) {
    uint8_t checksum = 0;
    if (nullptr == pdata) {
        return 0;
    }
    for (uint16_t i = 0; i < data_len; i++) {
        checksum += pdata[i];
    }
    checksum = ~checksum;

    return checksum;
}

uint16_t libfcn_v2::CheckSum1x2(uint8_t *pdata, uint16_t data_len) {
    uint16_t checksum = 0;
    if (nullptr == pdata) {
        return 0;
    }

    for (uint16_t i = 0; i < data_len; i++) {
        checksum += pdata[i];
    }
    checksum = ~checksum;

    return checksum;
}

uint32_t libfcn_v2::CheckSum1x4(uint8_t *pdata, uint16_t data_len) {
    uint32_t checksum = 0;
    if (nullptr == pdata) {
        return 0;
    }

    for (uint16_t i = 0; i < data_len; i++) {
        checksum += pdata[i];
    }
    checksum = ~checksum;

    return checksum;
}

uint32_t libfcn_v2::CheckSum4x4(uint8_t *pdata, uint16_t data_len) {
    uint32_t checksum = 0;
    if (nullptr == pdata) {
        return 0;
    }

    uint32_t* data = (uint32_t*)pdata;

    for (uint16_t i = 0; i < (data_len >> 2); i++) {
        checksum += data[i];
    }
    checksum = ~checksum;

    return checksum;
}