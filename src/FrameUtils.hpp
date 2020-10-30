//
// Created by sdong on 2019/11/11.
//

#ifndef LIBFCN_FRAMEUTILS_HPP
#define LIBFCN_FRAMEUTILS_HPP

#include <string>
#include "DataLinkLayer.hpp"

namespace libfcn_v2{

    std::string DataLinkFrameToString(DataLinkFrame& frame);

    std::string DataLinkFrameToStringCompact(DataLinkFrame& frame);

    bool DataLinkFrameCompare(DataLinkFrame& frame1, DataLinkFrame& frame2);

    bool DataLinkFramePayloadCompare(DataLinkFrame& frame1,
                                     DataLinkFrame& frame2);
}

#endif //LIBFCN_FRAMEUTILS_HPP
