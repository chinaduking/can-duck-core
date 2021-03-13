add_library(can_duck
        PubSub.cpp
        ClientRequestEv.cpp
        ParamServer.cpp
        FileTransfer.cpp
        Context.cpp
        DuckDebug.cpp)

target_link_libraries(can_duck ${emlib_LIBS})