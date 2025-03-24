module;
#include <exception>
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

} // namespace http
