include_directories(../../../src)

add_executable(sterm main.cpp)

target_link_libraries(sterm serial_port tracer)