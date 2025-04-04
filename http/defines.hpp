#pragma once

#include <cerrno>
#include <limits>
#include <source_location>
#include <stdexcept>

// Must be imported by the translation unit using this file
// import logger;
// import http;  // Can't import. Triggers a circular dependency error

#define HTTP_UU __attribute__((unused))

#define HTTP_LOG_GENERIC(type, fmt, ...)                                       \
  logger::log<http::exception>(type, std::numeric_limits<int>::min(),          \
                               std::source_location::current(),                \
                               fmt __VA_OPT__(, ) __VA_ARGS__)

#define HTTP_LOG_GENERIC_ERRNO(errno_v, type, fmt, ...)                        \
  logger::log<http::exception>(type, errno_v, std::source_location::current(), \
                               fmt __VA_OPT__(, ) __VA_ARGS__)

#define HTTP_LOG_FATAL(fmt, ...)                                               \
  HTTP_LOG_GENERIC(logger::type_t::fatal, fmt __VA_OPT__(, ) __VA_ARGS__)
#define HTTP_LOG_FATAL_ERRNO(fmt, ...)                                         \
  HTTP_LOG_GENERIC_ERRNO(errno, logger::type_t::fatal,                         \
                         fmt __VA_OPT__(, ) __VA_ARGS__)

#define HTTP_LOG_ERROR(fmt, ...)                                               \
  HTTP_LOG_GENERIC(logger::type_t::error, fmt __VA_OPT__(, ) __VA_ARGS__)
#define HTTP_LOG_ERROR_ERRNO(fmt, ...)                                         \
  HTTP_LOG_GENERIC_ERRNO(errno, logger::type_t::error,                         \
                         fmt __VA_OPT__(, ) __VA_ARGS__)

#define HTTP_LOG_WARN(fmt, ...)                                                \
  HTTP_LOG_GENERIC(logger::type_t::warning, fmt __VA_OPT__(, ) __VA_ARGS__)
#define HTTP_LOG_WARN_ERRNO(fmt, ...)                                          \
  HTTP_LOG_GENERIC_ERRNO(errno, logger::type_t::warning,                       \
                         fmt __VA_OPT__(, ) __VA_ARGS__)

#define HTTP_LOG_INFO(fmt, ...)                                                \
  HTTP_LOG_GENERIC(logger::type_t::info, fmt __VA_OPT__(, ) __VA_ARGS__)
#define HTTP_LOG_INFO_ERRNO(fmt, ...)                                          \
  HTTP_LOG_GENERIC_ERRNO(errno, logger::type_t::info,                          \
                         fmt __VA_OPT__(, ) __VA_ARGS__)
