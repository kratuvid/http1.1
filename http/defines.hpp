#pragma once

#include <cerrno>
#include <limits>
#include <source_location>

#ifdef NDEBUG
#define HTTP_TEST_CONDITIONAL_EXPR 0
#else
#define HTTP_TEST_CONDITIONAL_EXPR 1
#endif

#if HTTP_TEST_CONDITIONAL_EXPR == 1
#define HTTP_TEST_CONDITIONAL_EXPORT export
#else
#define HTTP_TEST_CONDITIONAL_EXPORT
#endif

#define HTTP_UU __attribute__((unused))

#define HTTP_LOG_GENERIC(type, fmt, ...)                                       \
  http::log::logger(type, std::numeric_limits<int>::min(),                     \
                    std::source_location::current(),                           \
                    fmt __VA_OPT__(, ) __VA_ARGS__)

#define HTTP_LOG_GENERIC_ERRNO(eno, type, fmt, ...)                            \
  http::log::logger(type, eno, std::source_location::current(),                \
                    fmt __VA_OPT__(, ) __VA_ARGS__)

#define HTTP_LOG_FATAL(fmt, ...)                                               \
  HTTP_LOG_GENERIC(http::log::type::fatal, fmt __VA_OPT__(, ) __VA_ARGS__);
#define HTTP_LOG_FATAL_ERRNO(fmt, ...)                                         \
  HTTP_LOG_GENERIC_ERRNO(errno, http::log::type::fatal,                        \
                         fmt __VA_OPT__(, ) __VA_ARGS__);

#define HTTP_LOG_ERROR(fmt, ...)                                               \
  HTTP_LOG_GENERIC(http::log::type::error, fmt __VA_OPT__(, ) __VA_ARGS__);
#define HTTP_LOG_ERROR_ERRNO(fmt, ...)                                         \
  HTTP_LOG_GENERIC_ERRNO(errno, http::log::type::error,                        \
                         fmt __VA_OPT__(, ) __VA_ARGS__);

#define HTTP_LOG_WARN(fmt, ...)                                                \
  HTTP_LOG_GENERIC(http::log::type::warning, fmt __VA_OPT__(, ) __VA_ARGS__);
#define HTTP_LOG_WARN_ERRNO(fmt, ...)                                          \
  HTTP_LOG_GENERIC_ERRNO(errno, http::log::type::warning,                      \
                         fmt __VA_OPT__(, ) __VA_ARGS__);

#define HTTP_LOG_INFO(fmt, ...)                                                \
  HTTP_LOG_GENERIC(http::log::type::info, fmt __VA_OPT__(, ) __VA_ARGS__);
#define HTTP_LOG_INFO_ERRNO(fmt, ...)                                          \
  HTTP_LOG_GENERIC_ERRNO(errno, http::log::type::info,                         \
                         fmt __VA_OPT__(, ) __VA_ARGS__);
