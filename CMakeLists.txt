cmake_minimum_required(VERSION 3.31)

# Variables
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_COLOR_DIAGNOSTICS YES)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

# User variable
set(is_debug NO)
if(CMAKE_BUILD_TYPE MATCHES "^(Debug|RelWithDebInfo)$")
  set(is_debug YES)
endif()

set(is_release YES)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(is_release NO)
endif()

# Options
set(common_options "")
set(enable_sanitizer YES)

# Conditional flags
if(is_debug)
  if(enable_sanitizer)
    list(APPEND common_options "-fsanitize=address,undefined")
  endif()

  add_compile_options(-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_BACKTRACE)
 	link_libraries(stdc++exp)
endif()

# Project wide options
add_compile_options(${common_options}
  -march=native -pipe
  -Wall -Wextra -Wpedantic -Werror
)
add_link_options(${common_options})

# Project
project(http1.1 LANGUAGES CXX)

add_library(http SHARED)
target_sources(http PUBLIC FILE_SET CXX_MODULES BASE_DIRS http
  FILES
  http/http.cppm http/util.cppm http/exception.cppm
  http/socket.cppm http/log.cppm
)

add_executable(hello hello.cpp)
target_link_libraries(hello http)
