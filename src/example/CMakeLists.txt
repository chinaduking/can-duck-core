include_directories(${PROJECT_SOURCE_DIR}/src/libfcn)
include_directories(${PROJECT_SOURCE_DIR}/src/utils)

add_executable(RTO_DualUartPC RTO_DualUartPC/main.cpp
        idl_gen/test_ServoDict.cpp)
target_link_libraries(RTO_DualUartPC fcn_v2 MyCppUtils)


add_executable(SVO_DualUartPC SVO_DualUartPC/main.cpp)
target_link_libraries(SVO_DualUartPC fcn_v2)
