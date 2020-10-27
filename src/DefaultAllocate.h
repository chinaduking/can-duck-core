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
     * 最多支持的通信端口数目
     * */
    #define MAX_COM_PORT_NUM 4

    /*
     * 最多支持的本地节点数目
     * */
    #define MAX_LOCAL_NODE_NUM 6


    /*
     * 最多支持的请求任务数
     * */
    #define MAX_REQ_TASK_NUM 16


    /*
     * 是否使用事件循环管理请求
     **/
//#define USE_EVLOOP

}

#endif //LIBFCN_V2_DEFAULTALLOCATE_H
