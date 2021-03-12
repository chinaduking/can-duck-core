
set(UT_DEFAULT_LIBS ${emlib_LIBS} GTest::gmock_main  GTest::gtest  GTest::gtest_main)

add_executable(testDataObjects SerDesDict.cpp ServoPubMsg.cpp
        )
target_link_libraries(testDataObjects ${UT_DEFAULT_LIBS})

add_executable(testPubSub
        PubSub.cpp
        ServoPubMsg.cpp
        ${PROJECT_SOURCE_DIR}/src/PubSub.cpp
        ${PROJECT_SOURCE_DIR}/src/ParamServer.cpp
        ${PROJECT_SOURCE_DIR}/src/ParamServerRequestEv.cpp
        ${PROJECT_SOURCE_DIR}/src/NetworkLayer.cpp)

target_link_libraries(testPubSub ${UT_DEFAULT_LIBS} )

add_executable(testParamServer
        ParamServer.cpp
        ServoPubMsg.cpp
        ${PROJECT_SOURCE_DIR}/src/PubSub.cpp
        ${PROJECT_SOURCE_DIR}/src/ParamServer.cpp
        ${PROJECT_SOURCE_DIR}/src/ParamServerRequestEv.cpp
        ${PROJECT_SOURCE_DIR}/src/NetworkLayer.cpp)

target_link_libraries(testParamServer ${UT_DEFAULT_LIBS} )


#add_executable(testServiceObject ParamServer.cpp  ServoPubMsg.cpp)
#target_link_libraries(testServiceObject fcn_v2 ${UT_DEFAULT_LIBS})
#
#
#
#add_executable(testDataLinkLayer DataLinkLayer.cpp)
#target_link_libraries(testDataLinkLayer fcn_v2 ${UT_DEFAULT_LIBS})
#
#
#
#add_executable(testNetworkLayer NetworkLayer.cpp)
#
#target_link_libraries(testNetworkLayer fcn_v2 ${UT_DEFAULT_LIBS})
#
#
#add_executable(testFlatbuffers testFlatbuffers.cpp)
#find_package(Flatbuffers CONFIG REQUIRED)
#target_link_libraries(testFlatbuffers PRIVATE flatbuffers::flatbuffers)
#
