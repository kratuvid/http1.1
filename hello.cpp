#include <print>
#include <exception>
#include "defines.hpp"
import http;

auto main() -> int {
  try {
    return http::testing();
  } catch (std::exception &e) {
    std::println(stderr, "Exception: {}", e.what());
    return 1;
  }
}
