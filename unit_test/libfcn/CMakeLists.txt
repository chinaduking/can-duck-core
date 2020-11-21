
set(UT_DEFAULT_LIBS tracer GTest::gmock_main  GTest::gtest  GTest::gtest_main)

add_executable(testDataObjects SerDesDict.cpp ServoPubMsg.cpp
        )
target_link_libraries(testDataObjects ${UT_DEFAULT_LIBS})



add_executable(testRealtimeObject PubSub.cpp ServoPubMsg.cpp)
target_link_libraries(testRealtimeObject fcn_v2 ${UT_DEFAULT_LIBS})



add_executable(testServiceObject ParamServer.cpp
        )
target_link_libraries(testServiceObject fcn_v2 ${UT_DEFAULT_LIBS})



add_executable(testDataLinkLayer DataLinkLayer.cpp
        ../../src/libfcn/DataLinkLayer.cpp
        )
target_link_libraries(testDataLinkLayer fcn_v2 ${UT_DEFAULT_LIBS})



add_executable(testNetworkLayer NetworkLayer.cpp
        ../../src/utils/os_only/HostSerial.cpp)
target_link_libraries(testNetworkLayer fcn_v2 ${UT_DEFAULT_LIBS})
