//
// Created by sdong on 2020/10/15.
//

#include "ServiceObject.hpp"
#include "OperationCode.hpp"
using namespace libfcn_v2;

/*将缓冲区内容写入参数表（1个项目），写入数据长度必须匹配元信息中的数据长度*/
obj_size_t ServiceObjectDict::singleWrite(obj_idx_t index, uint8_t *data,
                                          obj_size_t len){

    if(index > obj_dict.size()){
        /* 仅做写保护，不使程序assert failed崩溃：
         * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
         * 可记录错误log
         * */
        return 1;
    }

    auto p_obj = obj_dict[index];

    USER_ASSERT(p_obj != nullptr);

    /* 单数据写入，要求长度要求必须匹配 */
    if(index > obj_dict.size()){
        /* 仅做写保护，不使程序assert failed崩溃：
         * 外界输入（index为通信接收的数据）的异常不应使程序崩溃
         * 可记录错误log
         * */
        return 1;
    }


    utils::memcpy(p_obj->getDataPtr(), data,
                  p_obj->data_size);

    auto callback = p_obj->callback;

    if(callback != nullptr){
        callback->callback(p_obj->getDataPtr(), 0);
    }


    return 0;
}