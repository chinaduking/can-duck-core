//
// Created by sdong on 2020/10/29.
//

#include "test_ServoDict.hpp"


using namespace libfcn_v2;

namespace fcnmsg{
    test_ServoPubSubDict_C  test_ServoPubSubDict;

    void * test_ServoPubSubDict_C::createBuffer() {
        return new test_ServoPubSubDict_C::Buffer;
    }

}