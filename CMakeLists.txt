cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)
project(can-duck-core)

#------------Set Compiler-------------------
# export CC=/usr/bin/clang
# export CXX=/usr/bin/clang++

message("---> CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

#------------Set Build / Debug Mode ---------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message("---->Build type not defined, set to ${CMAKE_BUILD_TYPE} by default.")
else()
    message("----> CMAKE_BUILD_TYPE is " ${CMAKE_BUILD_TYPE})
endif()

#------------Set Compile Flags-------------------

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -std=c99 -funroll-loops")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -funroll-loops -fpic \
    -Wall -Wno-sign-compare -Wno-unused-private-field -Wno-writable-strings \
    -Werror=return-type  -Wno-unused-variable\
     -pthread -ffast-math")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -std=c99 -funroll-loops")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O3")
endif()

#------------Set Output Path-------------------
if( (${CMAKE_SYSTEM_NAME} STREQUAL Linux) OR (${CMAKE_SYSTEM_NAME} STREQUAL Darwin))
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") #arm
#        set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/linux_arm)
#    elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64") #x86_64
#        set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/linux_x86_64)
#    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Android)
    add_definitions("-DPLATFORM_ANDROID")
    if(${CMAKE_ANDROID_ARCH_ABI} MATCHES "arm64-v8a") #arm64_android
        set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs/arm64)
    elseif(${CMAKE_ANDROID_ARCH_ABI} MATCHES "x86_64") #x64_android
        set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs/x64)
    endif()
endif()
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#------------Add Definetions In Source Code-------------------
add_definitions(-DPROJECT_ROOT="${PROJECT_SOURCE_DIR}/")

execute_process(COMMAND "git" "describe" "--always" "--dirty"
                OUTPUT_VARIABLE GIT_COMMAND_OUTPUT)
string(REPLACE "\n" "" GIT_COMMAND_OUTPUT ${GIT_COMMAND_OUTPUT})
message("-----> git describe: " ${GIT_COMMAND_OUTPUT})

add_definitions(-DGIT_DESCRIBE_STR="${GIT_COMMAND_OUTPUT}")

#------------Check Target Platform---------------
# CMAKE_SYSTEM_NAME may be: Linux, Windows, Darwin(MacOSX), Android
message("----> CMAKE_SYSTEM_NAME is: " ${CMAKE_SYSTEM_NAME})
message("----> CMAKE_SYSTEM_PROCESSOR is: " ${CMAKE_SYSTEM_PROCESSOR})

add_definitions(-DSYSTYPE_FULL_OS)



#if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
find_package(GTest CONFIG REQUIRED)

#endif()

#---------------------------------------------
#------------ emlib  -------------------
#---------------------------------------------

add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/emlib-cpp)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/emlib-cpp/src)
set(emlib_LIBS emlib)

#------------Add Submodule---------------
include_directories(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(${PROJECT_SOURCE_DIR}/unit_test)