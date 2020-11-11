add_library(tracer Tracer.cpp os_only/HostIODeviceWrapper.cpp)

add_library(serial_port os_only/HostSerial.cpp)
target_link_libraries(serial_port tracer)

