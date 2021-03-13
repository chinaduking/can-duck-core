
set(UT_DEFAULT_LIBS ${emlib_LIBS} GTest::gmock_main  GTest::gtest  GTest::gtest_main)

add_executable(testDataObjects SerDesDict.cpp ServoDict.cpp
        )
target_link_libraries(testDataObjects ${UT_DEFAULT_LIBS})

add_executable(testPubSub
        PubSub.cpp
        ServoDict.cpp
        ${PROJECT_SOURCE_DIR}/src/Message.cpp
        ${PROJECT_SOURCE_DIR}/src/DuckDebug.cpp)

target_link_libraries(testPubSub ${UT_DEFAULT_LIBS} )

add_executable(testParamServer
        ParamServer.cpp
        ServoDict.cpp
        ${PROJECT_SOURCE_DIR}/src/Message.cpp
        ${PROJECT_SOURCE_DIR}/src/Service.cpp
        ${PROJECT_SOURCE_DIR}/src/ClientRequestEv.cpp
        ${PROJECT_SOURCE_DIR}/src/DuckDebug.cpp
        )

target_link_libraries(testParamServer ${UT_DEFAULT_LIBS} )


add_executable(testNode Context.cpp ServoDict.cpp)

target_link_libraries(testNode can_duck ${UT_DEFAULT_LIBS} )