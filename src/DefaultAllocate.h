//
// Created by sdong on 2020/10/21.
//

#ifndef LIBFCN_V2_DEFAULTALLOCATE_H
#define LIBFCN_V2_DEFAULTALLOCATE_H

namespace libfcn_v2{

#ifndef FCN_ALLOCATE_FRAME_NUM
    #define FCN_ALLOCATE_FRAME_NUM 20
#endif


#ifndef MAX_COM_PORT_NUM
#define MAX_COM_PORT_NUM 4
#endif

/*
 * 最多支持的本地节点数目
 * */
#define MAX_LOCAL_NODE 6


#define USE_EVLOOP

}

#endif //LIBFCN_V2_DEFAULTALLOCATE_H
