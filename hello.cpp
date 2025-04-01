#include <exception>
#include <print>
#include <vector>
#include "defines.hpp"

import http;

auto main(UU int argc, UU char **argv) -> int {
  try {
    std::vector<std::string_view> vargv(argc-1);
    std::copy(argv + 1, argv + argc, vargv.begin());
    return http::testing(vargv);
  } catch (std::exception &e) {
    std::println(stderr, "Exception: {}", e.what());
    return 1;
  }
}
