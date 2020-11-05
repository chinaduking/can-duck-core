//
// Created by sdong on 2020/10/21.
//

#ifndef LIBFCN_V2_DEFAULTALLOCATE_H
#define LIBFCN_V2_DEFAULTALLOCATE_H

namespace libfcn_v2{
    /*
     * 最多可在堆上创建的数据帧
     **/
    #define FCN_ALLOCATE_FRAME_NUM 4

    /*
     *
     **/
    #define DATALINK_MAX_TRANS_UNIT  64

    /*
     * 最多支持的通信端口数目
     * */
    #define MAX_COM_PORT_NUM 4

    /*
     * 最多支持的本地节点数目
     **/
    #define MAX_LOCAL_NODE_NUM 6

    /*
     * 是否使用事件循环管理请求
     **/
    #define USE_REQUEST_EVLOOP


    /*
     * 一个客户端可以同时发起的请求
     **/
    #define CLIENT_MAX_REQ_NUM 5


    /*
     *
     **/
    #define ENABLE_LOG
}

#endif //LIBFCN_V2_DEFAULTALLOCATE_H
