cmake_minimum_required(VERSION 3.31)

project(logger LANGUAGES CXX)
include(GoogleTest)

add_library(logger SHARED)
target_sources(logger PUBLIC FILE_SET CXX_MODULES
  FILES logger.cppm
)

add_executable(logger_tests tests.cpp)
target_link_libraries(logger_tests gtest_main gtest logger)
gtest_discover_tests(logger_tests)
