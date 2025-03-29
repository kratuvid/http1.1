cmake_minimum_required(VERSION 3.31)

project(http LANGUAGES CXX)
include(GoogleTest)

add_library(http SHARED)
target_sources(http PUBLIC FILE_SET CXX_MODULES
  FILES
  http.cppm util.cppm exception.cppm socket.cppm
)
target_link_libraries(http logger)

add_executable(tests tests.cpp)
target_link_libraries(tests gtest_main gtest http)
gtest_discover_tests(tests)
