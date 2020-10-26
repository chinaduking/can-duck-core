include_directories(${PROJECT_SOURCE_DIR}/unit_test)

add_executable(testTracer Tracer.cpp
        ${PROJECT_SOURCE_DIR}/src/utils/Tracer.cpp
        ${PROJECT_SOURCE_DIR}/src/utils/HostIODeviceWrapper.cpp)

target_link_libraries(testTracer  GTest::GTest GTest::Main)


add_executable(testArray Array.cpp)
target_link_libraries(testArray  GTest::GTest GTest::Main)


add_executable(testStaticSet vector_s.cpp)
target_link_libraries(testStaticSet  GTest::GTest GTest::Main)


add_executable(testPosixSerial PosixSerial.cpp
        ${PROJECT_SOURCE_DIR}/src/utils/PosixSerial.cpp)

target_link_libraries(testPosixSerial  GTest::GTest GTest::Main)


add_executable(testESharedPtr ESharedPtr.cpp)

target_link_libraries(testESharedPtr fcn_v2 GTest::GTest GTest::Main)


add_executable(testEHeap EHeap.cpp)

target_link_libraries(testEHeap  GTest::GTest GTest::Main)


add_executable(testLinkedList LinkedList.cpp)

target_link_libraries(testLinkedList  GTest::GTest GTest::Main)

