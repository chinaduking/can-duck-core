add_executable(testRealtimeObject RealtimeObject.cpp
        ${PROJECT_SOURCE_DIR}/src/RealtimeObject.cpp)

target_link_libraries(testRealtimeObject  GTest::GTest GTest::Main)


add_executable(testDataLinkLayer DataLinkLayer.cpp
        ${PROJECT_SOURCE_DIR}/src/DataLinkLayer.cpp
        ${PROJECT_SOURCE_DIR}/src/utils/DataVerify.cpp
        ${PROJECT_SOURCE_DIR}/src/utils/PosixSerial.cpp
        ${PROJECT_SOURCE_DIR}/src/utils/EHeap.cpp)

target_link_libraries(testDataLinkLayer  GTest::GTest GTest::Main)


add_executable(testNetworkLayer NetworkLayer.cpp
        ${PROJECT_SOURCE_DIR}/src/utils/PosixSerial.cpp)

target_link_libraries(testNetworkLayer fcn_v2 GTest::GTest GTest::Main)
