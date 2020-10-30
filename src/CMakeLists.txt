add_library(fcn_v2
        DataLinkLayer.cpp
        NetworkLayer.cpp
        RealtimeObject.cpp
        ServiceObject.cpp
        TracerSingleton.cpp
        FrameUtils.cpp

        utils/DataVerify.cpp
        utils/Tracer.cpp
        utils/HostIODeviceWrapper.cpp)