#pragma once

#include <cerrno>
#include <limits>
#include <source_location>

#define QUIT_THROWING(fmt, ...)                                                \
  quit_throwing(std::numeric_limits<int>::min(),                               \
                std::source_location::current(),                               \
                fmt __VA_OPT__(, ) __VA_ARGS__)

#define QUIT_ERRNO_THROWING(fmt, ...)                                          \
  quit_throwing(errno, std::source_location::current(),                        \
                fmt __VA_OPT__(, ) __VA_ARGS__)
