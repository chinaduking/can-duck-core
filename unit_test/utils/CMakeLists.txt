set(UT_DEFAULT_LIBS tracer GTest::gmock_main  GTest::gtest  GTest::gtest_main)

include_directories(${PROJECT_SOURCE_DIR}/unit_test)


add_executable(testTracer Tracer.cpp)
target_link_libraries(testTracer ${UT_DEFAULT_LIBS})


add_executable(testArray Array.cpp WinSerial.cpp)
target_link_libraries(testArray ${UT_DEFAULT_LIBS})

add_executable(testCppUtils CppUtils.cpp)
target_link_libraries(testCppUtils ${UT_DEFAULT_LIBS})


add_executable(testVector vector_s.cpp)
target_link_libraries(testVector ${UT_DEFAULT_LIBS})

add_executable(testPosixSerial PosixSerial.cpp)
target_link_libraries(testPosixSerial serial_port ${UT_DEFAULT_LIBS})

add_executable(testESharedPtr ESharedPtr.cpp)
target_link_libraries(testESharedPtr fcn_v2 ${UT_DEFAULT_LIBS})


add_executable(testEHeap EHeap.cpp)
target_link_libraries(testEHeap  ${UT_DEFAULT_LIBS})


add_executable(testLinkedList LinkedList.cpp)
target_link_libraries(testLinkedList ${UT_DEFAULT_LIBS})


add_executable(testMemMap MemMap.cpp)
target_link_libraries(testMemMap ${UT_DEFAULT_LIBS})


add_executable(testEvloop EventLoopThreading.cpp
        ${PROJECT_SOURCE_DIR}/src/libfcn/DataLinkLayer.cpp)
target_link_libraries(testEvloop fcn_v2 ${UT_DEFAULT_LIBS})


add_executable(testQueue queue_s.cpp)
target_link_libraries(testQueue ${UT_DEFAULT_LIBS})

add_executable(testWinSerial WinSerial.cpp)

