//
// Created by sdong on 2020/10/21.
//

#ifndef can_duck_DEFAULTALLOCATE_H
#define can_duck_DEFAULTALLOCATE_H

namespace can_duck{
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
     *
     **/
    #define ENABLE_LOG
}

#endif //can_duck_DEFAULTALLOCATE_H
