include_directories(${PROJECT_SOURCE_DIR}/src/libfcn)

add_executable(testDataObjects SerDesDict.cpp
        test_ServoDict.cpp
        )
target_link_libraries(testDataObjects  GTest::GTest GTest::Main)



add_executable(testRealtimeObject PubSub.cpp
        test_ServoDict.cpp
        )
target_link_libraries(testRealtimeObject fcn_v2 MyCppUtils GTest::GTest
        GTest::Main)



add_executable(testServiceObject ParamServer.cpp
        test_ServoDict.cpp
        )
target_link_libraries(testServiceObject  fcn_v2 MyCppUtils  GTest::GTest
        GTest::Main)



add_executable(testDataLinkLayer DataLinkLayer.cpp
        ../../src/libfcn/DataLinkLayer.cpp
        )
target_link_libraries(testDataLinkLayer MyCppUtils GTest::GTest GTest::Main)



add_executable(testNetworkLayer NetworkLayer.cpp
        ../../src/utils/os_only/PosixSerial.cpp)
target_link_libraries(testNetworkLayer fcn_v2 GTest::GTest GTest::Main)
