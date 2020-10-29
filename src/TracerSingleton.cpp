//
// Created by sdong on 2020/10/29.
//

#include "TracerSingleton.hpp"

using namespace libfcn_v2;


utils::Tracer* TracerSingleton::tracer = nullptr;



utils::Tracer* TracerSingleton::getInstance(){
    if(tracer == nullptr){
        tracer = new utils::Tracer(true);

        tracer->setFilter(utils::Tracer::INFO);
    }

    return tracer;
}
