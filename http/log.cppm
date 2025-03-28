module;
#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <limits>
#include <print>
#include <source_location>

#include "defines.hpp"
export module http:log;

import :exception;

namespace http {

HTTP_TEST_CONDITIONAL_EXPORT
class log {
public:
  enum class type { info, warning, error, fatal };

public:
  template <typename... Args>
  [[gnu::noinline]]
  static auto logger(log::type type, int errno_val, std::source_location sl,
                     std::format_string<Args...> fmt, Args &&...args) -> void {
    if (!m_is_terminal_tested) {
      if (isatty(2)) {
        const auto term_raw = std::getenv("TERM");
        if (term_raw) {
          const auto term = std::string_view(term_raw);
          const auto npos = decltype(term)::npos;
          if (term.find("color") != npos || term.find("direct") != npos) {
            m_disable_escape_codes = false;
          }
        }
      }
      m_is_terminal_tested = true;
    }

    const auto stripped_file_name{
        std::filesystem::path(sl.file_name()).filename()};

    const auto brevity_n = static_cast<int>(m_brevity);

    auto explanation{std::format("{}{}{}: ", _type2color(type), _type2str(type),
                                 _reset_attribs())};
    if (brevity_n > static_cast<int>(brevity_t::strip_file))
      explanation += std::format("{}: ", stripped_file_name.c_str(), sl.line());
    if (brevity_n > static_cast<int>(brevity_t::strip_line))
      explanation += std::format("{}: ", sl.line());
    if (brevity_n > static_cast<int>(brevity_t::strip_function))
      explanation += std::format("{}:\n  ", sl.function_name());
    explanation += std::format(fmt, std::forward<Args>(args)...);

    /* auto explanation{std::format(
    "{}{}{}: {}({}): {}:\n  {}", _type2color(type), _type2str(type),
    _reset_attribs(), stripped_file_name.c_str(), sl.line(),
    sl.function_name(), std::format(fmt, std::forward<Args>(args)...))}; */

    if (errno_val != std::numeric_limits<int>::min())
      explanation += std::format(": {}", std::strerror(errno_val));

    if (type == log::type::fatal)
      throw http::exception(std::move(explanation));
    else
      std::println(std::cerr, "{}", explanation);
  }

  static auto force_disable_escape_codes() -> void {
    m_is_terminal_tested = true;
    m_disable_escape_codes = true;
  }

private:
  static auto _type2str(log::type t) -> const char * {
    switch (t) {
    case type::info:
      return "INFO";
    case type::warning:
      return "WARN";
    case type::error:
      return "ERROR";
    case type::fatal:
      return "FATAL";
    }
  };

#define _X_CSI "\x1b["
#define _X_FG _X_CSI "38;5;"

  static auto _type2color(log::type t) -> const char * {
    if (m_disable_escape_codes)
      return "";

    switch (t) {
    case type::info:
      return _X_FG "244m"; // bright grey
    case type::warning:
      return _X_FG "11m"; // bright yellow
    case type::error:
      return _X_FG "208m"; // bright orange
    case type::fatal:
      return _X_FG "9m"; // bright red
    }
  }

  static auto _reset_attribs() -> const char * {
    if (m_disable_escape_codes)
      return "";

    return _X_CSI "0m";
  }

#undef _X_FG
#undef _X_CSI

private:
  inline static bool m_is_terminal_tested = false;
  inline static bool m_disable_escape_codes = true;

public:
  enum class brevity_t {
    strip_file,
    strip_line,
    strip_function,
    verbose,
  };
  inline static brevity_t m_brevity{brevity_t::strip_file};
};

}; // namespace http
