add_library(fcn_v2
        DataLinkLayer.cpp
        NetworkLayer.cpp
        PubSub.cpp
        ParamServer.cpp
        Log.cpp
        FrameUtils.cpp

        utils/DataVerify.cpp
        utils/Tracer.cpp
        utils/HostIODeviceWrapper.cpp)