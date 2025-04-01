cmake_minimum_required(VERSION 3.31)

project(terminfo LANGUAGES CXX)
include(GoogleTest)

add_library(terminfo SHARED)
target_sources(terminfo PUBLIC FILE_SET CXX_MODULES
  FILES terminfo.cppm exception.cppm
)
target_link_libraries(terminfo)

add_executable(max_colors max_colors.cpp)
target_link_libraries(max_colors terminfo)

add_executable(terminfo_tests tests.cpp)
target_link_libraries(terminfo_tests gtest_main gtest terminfo)
gtest_discover_tests(terminfo_tests)
