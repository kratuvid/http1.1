module;
#include <exception>
#include <string>
export module terminfo:exception;

namespace terminfo {

class exception : public std::exception {
public:
  exception(std::string_view &expl) : explanation(std::string("terminfo: ") + expl.data()) {}
  exception(std::string &&expl) : explanation(std::string("terminfo: ") + expl) {}
  exception(exception const &) = delete;
  exception(exception &&) = delete;
  exception &operator=(exception const &) = delete;
  exception &operator=(exception &&) = delete;

  const char *what() const noexcept override { return explanation.c_str(); }

private:
  std::string explanation;
};

} // namespace terminfo
