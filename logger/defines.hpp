#pragma once

#include <cerrno>
#include <limits>
#include <source_location>
#include <stdexcept>

#define LOG_GENERIC(type, fmt, ...)                                            \
  logger::log<std::runtime_error>(type, std::numeric_limits<int>::min(),       \
                                  std::source_location::current(),             \
                                  fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_GENERIC_ERRNO(errno_v, type, fmt, ...)                             \
  logger::log<std::runtime_error>(type, errno_v,                               \
                                  std::source_location::current(),             \
                                  fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_FATAL(fmt, ...)                                                    \
  LOG_GENERIC(logger::type_t::fatal, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_FATAL_ERRNO(fmt, ...)                                              \
  LOG_GENERIC_ERRNO(errno, logger::type_t::fatal,                              \
                    fmt __VA_OPT__(, ) __VA_ARGS__);

#define LOG_ERROR(fmt, ...)                                                    \
  LOG_GENERIC(logger::type_t::error, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_ERROR_ERRNO(fmt, ...)                                              \
  LOG_GENERIC_ERRNO(errno, logger::type_t::error,                              \
                    fmt __VA_OPT__(, ) __VA_ARGS__);

#define LOG_WARN(fmt, ...)                                                     \
  LOG_GENERIC(logger::type_t::warning, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_WARN_ERRNO(fmt, ...)                                               \
  LOG_GENERIC_ERRNO(errno, logger::type_t::warning,                            \
                    fmt __VA_OPT__(, ) __VA_ARGS__);

#define LOG_INFO(fmt, ...)                                                     \
  LOG_GENERIC(logger::type_t::info, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_INFO_ERRNO(fmt, ...)                                               \
  LOG_GENERIC_ERRNO(errno, logger::type_t::info,                               \
                    fmt __VA_OPT__(, ) __VA_ARGS__);
