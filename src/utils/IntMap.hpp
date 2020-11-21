//
// Created by sdong on 2020/11/21.
//

#ifndef LIBFCN_INTMAP_HPP
#define LIBFCN_INTMAP_HPP
#include "LinkedList.hpp"

namespace utils{

    template<typename T>
    class IntMap{
    public:
        struct KeyVal{
            int key;
            T val;
        };

        IntMap() = default;

        LinkedList<KeyVal> list;
    };

}


#endif //LIBFCN_INTMAP_HPP
