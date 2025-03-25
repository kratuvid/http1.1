cmake_minimum_required(VERSION 3.31)

project(http LANGUAGES CXX)
include(GoogleTest)

add_library(http SHARED)
target_sources(http PUBLIC FILE_SET CXX_MODULES
  FILES
  http.cppm util.cppm exception.cppm socket.cppm log.cppm
)

add_executable(test test.cpp)
target_link_libraries(test gtest_main gtest http)
gtest_discover_tests(test)

