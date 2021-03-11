add_library(fcn_v2
        DataLinkLayer.cpp
        NetworkLayer.cpp
        PubSub.cpp
        ParamServer.cpp
        ParamServerRequestEv.cpp
        MultiFrame.cpp
        Node.cpp)

target_link_libraries(fcn_v2 ${emlib_LIBS})