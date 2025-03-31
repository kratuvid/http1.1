#pragma once

#include <cerrno>
#include <limits>
#include <source_location>
#include <stdexcept>

import logger;

#define TERMINFO_LOG_GENERIC(type, fmt, ...)                                   \
  logger::log<terminfo::exception>(type, std::numeric_limits<int>::min(),      \
                                   std::source_location::current(),            \
                                   fmt __VA_OPT__(, ) __VA_ARGS__)

#define TERMINFO_LOG_GENERIC_ERRNO(errno_v, type, fmt, ...)                    \
  logger::log<terminfo::exception>(type, errno_v,                              \
                                   std::source_location::current(),            \
                                   fmt __VA_OPT__(, ) __VA_ARGS__)

#define TERMINFO_LOG_FATAL(fmt, ...)                                           \
  TERMINFO_LOG_GENERIC(logger::type_t::fatal, fmt __VA_OPT__(, ) __VA_ARGS__);
#define TERMINFO_LOG_FATAL_ERRNO(fmt, ...)                                     \
  TERMINFO_LOG_GENERIC_ERRNO(errno, logger::type_t::fatal,                     \
                             fmt __VA_OPT__(, ) __VA_ARGS__);

#define TERMINFO_LOG_ERROR(fmt, ...)                                           \
  TERMINFO_LOG_GENERIC(logger::type_t::error, fmt __VA_OPT__(, ) __VA_ARGS__);
#define TERMINFO_LOG_ERROR_ERRNO(fmt, ...)                                     \
  TERMINFO_LOG_GENERIC_ERRNO(errno, logger::type_t::error,                     \
                             fmt __VA_OPT__(, ) __VA_ARGS__);

#define TERMINFO_LOG_WARN(fmt, ...)                                            \
  TERMINFO_LOG_GENERIC(logger::type_t::warning, fmt __VA_OPT__(, ) __VA_ARGS__);
#define TERMINFO_LOG_WARN_ERRNO(fmt, ...)                                      \
  TERMINFO_LOG_GENERIC_ERRNO(errno, logger::type_t::warning,                   \
                             fmt __VA_OPT__(, ) __VA_ARGS__);

#define TERMINFO_LOG_INFO(fmt, ...)                                            \
  TERMINFO_LOG_GENERIC(logger::type_t::info, fmt __VA_OPT__(, ) __VA_ARGS__);
#define TERMINFO_LOG_INFO_ERRNO(fmt, ...)                                      \
  TERMINFO_LOG_GENERIC_ERRNO(errno, logger::type_t::info,                      \
                             fmt __VA_OPT__(, ) __VA_ARGS__);
