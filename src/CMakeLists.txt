add_library(can_duck
    Message.cpp
    ClientRequestEv.cpp
    Service.cpp
    FileTransfer.cpp
    Context.cpp
    DuckDebug.cpp)

target_link_libraries(can_duck ${emlib_LIBS})