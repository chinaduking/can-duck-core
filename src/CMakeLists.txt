add_library(fcn_v2
        DataLinkLayer.cpp
        NetworkLayer.cpp
        PubSub.cpp
        ParamServer.cpp
        MultiFrame.cpp

        Log.cpp
        Node.cpp

        utils/DataVerify.cpp
        utils/Tracer.cpp
        utils/HostIODeviceWrapper.cpp)