include_directories(../../src/libfcn)
include_directories(../../src/utils)

add_executable(pubsub_pc_host
        pubsub_pc/host.cpp
        idl_gen/ServoPubMsg.cpp)

target_link_libraries(pubsub_pc_host fcn_v2  serial_port tracer)

add_executable(pubsub_pc_servo
        pubsub_pc/servo.cpp
        idl_gen/ServoPubMsg.cpp)

target_link_libraries(pubsub_pc_servo fcn_v2  serial_port tracer)


add_executable(SVO_DualUartPC SVO_DualUartPC/main.cpp)
target_link_libraries(SVO_DualUartPC fcn_v2)
