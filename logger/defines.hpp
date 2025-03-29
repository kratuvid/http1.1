#pragma once

#include <cerrno>
#include <limits>
#include <source_location>
#include <stdexcept>

import logger;

using logger_generic_t = logger<std::runtime_error>;

#define LOG_GENERIC(type, fmt, ...)                                            \
  logger_generic_t::log(type, std::numeric_limits<int>::min(),                 \
                        std::source_location::current(),                       \
                        fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_GENERIC_ERRNO(eno, type, fmt, ...)                                 \
  logger_generic_t::log(type, eno, std::source_location::current(),            \
                        fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_FATAL(fmt, ...)                                                    \
  LOG_GENERIC(logger_generic_t::type_t::fatal, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_FATAL_ERRNO(fmt, ...)                                              \
  LOG_GENERIC_ERRNO(errno, logger_generic_t::type_t::fatal,                    \
                    fmt __VA_OPT__(, ) __VA_ARGS__);

#define LOG_ERROR(fmt, ...)                                                    \
  LOG_GENERIC(logger_generic_t::type_t::error, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_ERROR_ERRNO(fmt, ...)                                              \
  LOG_GENERIC_ERRNO(errno, logger_generic_t::type_t::error,                    \
                    fmt __VA_OPT__(, ) __VA_ARGS__);

#define LOG_WARN(fmt, ...)                                                     \
  LOG_GENERIC(logger_generic_t::type_t::warning,                               \
              fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_WARN_ERRNO(fmt, ...)                                               \
  LOG_GENERIC_ERRNO(errno, logger_generic_t::type_t::warning,                  \
                    fmt __VA_OPT__(, ) __VA_ARGS__);

#define LOG_INFO(fmt, ...)                                                     \
  LOG_GENERIC(logger_generic_t::type_t::info, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_INFO_ERRNO(fmt, ...)                                               \
  LOG_GENERIC_ERRNO(errno, logger_generic_t::type_t::info,                     \
                    fmt __VA_OPT__(, ) __VA_ARGS__);
