include_directories(../..)

add_executable(logview main.cpp)

target_link_libraries(logview serial_port tracer)