include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(logview main.cpp)

target_link_libraries(logview MyCppUtils)