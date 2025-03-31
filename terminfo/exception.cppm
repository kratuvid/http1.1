module;
#include <exception>
#include <string>
export module terminfo:exception;

namespace terminfo {

class exception : public std::exception {
public:
  exception(std::string_view &expl) : explanation(expl) {}
  exception(std::string &&expl) : explanation(expl) {}
  exception(exception const &) = delete;
  exception(exception &&) = delete;
  exception &operator=(exception const &) = delete;
  exception &operator=(exception &&) = delete;

  const char *what() const noexcept override { return explanation.c_str(); }

private:
  std::string explanation;
};

} // namespace terminfo
