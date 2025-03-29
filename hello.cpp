#include <print>
#include <exception>
#include <vector>
#include <ranges>
#include "defines.hpp"
import http;

auto main(int argc, char** argv) -> int {
  try {
    std::vector<std::string_view> vargv;
    for (int i : std::views::iota(1, argc))
      vargv.push_back(argv[i]);
    return http::testing(vargv);
  } catch (std::exception &e) {
    std::println(stderr, "Exception: {}", e.what());
    return 1;
  }
}
