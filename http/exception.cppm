module;
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <limits>
#include <source_location>
#include <string>
export module http:exception;

namespace http {

class exception : public std::exception {
public:
  exception(std::string_view what) : explanation(what) {}
  exception(std::string &&what) : explanation(what) {}

  exception(exception const &) = delete;
  exception(exception &&) = delete;
  exception &operator=(exception const &) = delete;
  exception &operator=(exception &&) = delete;

  const char *what() const noexcept override { return explanation.c_str(); }

private:
  std::string explanation;
};

template <typename... Args>
[[gnu::noinline]]
auto quit_throwing(int errno_val, std::source_location sl,
                   std::format_string<Args...> fmt, Args &&...args) -> void {
  const auto stripped_file_name{
      std::filesystem::path(sl.file_name()).filename()};

  auto explanation{std::format("{}({}): {}:\n  {}", stripped_file_name.c_str(),
                               sl.line(), sl.function_name(),
                               std::format(fmt, std::forward<Args>(args)...))};

  if (errno_val != std::numeric_limits<int>::min())
    explanation += std::format(": {}", std::strerror(errno_val));

  throw http::exception(std::move(explanation));
}

} // namespace http
