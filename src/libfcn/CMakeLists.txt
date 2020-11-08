include_directories(${PROJECT_SOURCE_DIR}/src/utils)


add_library(fcn_v2
        DataLinkLayer.cpp
        NetworkLayer.cpp
        PubSub.cpp
        ParamServer.cpp
        ParamServerRequestEv.cpp
        MultiFrame.cpp
        DataVerify.cpp
        Node.cpp)

target_link_libraries(fcn_v2 tracer serial_port)