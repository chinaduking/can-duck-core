include_directories(${PROJECT_SOURCE_DIR}/src/utils)


add_library(fcn_v2
        DataLinkLayer.cpp
        NetworkLayer.cpp
        PubSub.cpp
        ParamServer.cpp
        ParamServerRequestEv.cpp
        MultiFrame.cpp

        Log.cpp
        Node.cpp

        ../utils/DataVerify.cpp
        ../utils/Tracer.cpp
        ../utils/HostIODeviceWrapper.cpp)