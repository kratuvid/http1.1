module;
#include <unistd.h>

#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <limits>
#include <print>
#include <source_location>
export module logger;

export template <typename Exception>
  requires(std::is_base_of_v<std::exception, Exception>)
class logger {
public:
  enum class type_t { info, warning, error, fatal };
  enum class brevity_t {
    strip_file,
    strip_line,
    strip_function,
    verbose,
  };

public:
  template <typename... Args>
  [[gnu::noinline]]
  static auto log(type_t type, int errno_val, std::source_location sl,
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

    if (errno_val != std::numeric_limits<int>::min())
      explanation += std::format(": {}", std::strerror(errno_val));

    if (type == type_t::fatal)
      throw Exception(std::move(explanation));
    else
      std::println(std::cerr, "{}", explanation);
  }

  static auto force_disable_escape_codes() -> void {
    m_is_terminal_tested = true;
    m_disable_escape_codes = true;
  }

private:
  static auto _type2str(type_t t) -> const char * {
    switch (t) {
    case type_t::info:
      return "INFO";
    case type_t::warning:
      return "WARN";
    case type_t::error:
      return "ERROR";
    case type_t::fatal:
      return "FATAL";
    }
  };

#define _X_CSI "\x1b["
#define _X_FG _X_CSI "38;5;"

  static auto _type2color(type_t t) -> const char * {
    if (m_disable_escape_codes)
      return "";

    switch (t) {
    case type_t::info:
      return _X_FG "244m"; // bright grey
    case type_t::warning:
      return _X_FG "11m"; // bright yellow
    case type_t::error:
      return _X_FG "208m"; // bright orange
    case type_t::fatal:
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
  inline static brevity_t m_brevity{brevity_t::strip_file};
};
