//
// Created by sdong on 2020/10/28.
//

#include "utils/MemMap.hpp"
#include "TestUtils.hpp"


using namespace utils;

MemMap pint_map(100);

struct PImtMap{
    static inline int add(void* v_ptr){
        return pint_map.add(v_ptr);
    }

    static inline void* query(int v_ptr){
        return pint_map.query(v_ptr);
    }
};

typedef vptr<int, uint8_t, PImtMap> IntVPtr;

TEST(MemMap, map){

    cout << sizeof(IntVPtr) << endl;

    int* p_int = new int;
    *p_int = 250;


    IntVPtr vp_int(p_int);

    int* p_int_query = &(*vp_int);

    cout << *p_int_query << endl;

    ASSERT_EQ(p_int_query, p_int);
//    auto v_pint = pint_map.add(p_int);

}

