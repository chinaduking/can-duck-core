//
// Created by sdong on 2020/10/29.
//

#include "Log.hpp"

using namespace libfcn_v2;


utils::Tracer* Log::tracer = nullptr;



utils::Tracer* Log::getInstance(){
    if(tracer == nullptr){
        tracer = new utils::Tracer(true);

        tracer->setFilter(utils::Tracer::INFO);
    }

    return tracer;
}
