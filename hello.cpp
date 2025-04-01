#include "defines.hpp"
#include <exception>
#include <memory>
#include <print>
#include <vector>

import terminfo;
import http;

auto main(UU int argc, UU char **argv) -> int {
  try {
#if 1
    auto ti = std::make_unique<terminfo::reader>();
    auto ti_legacy = std::make_unique<terminfo::reader>(true, "xterm");
    auto ti_konsole = std::make_unique<terminfo::reader>(true, "konsole-256color");
#else
    terminfo::reader ti;
    terminfo::reader ti_legacy(true, "xterm");
    terminfo::reader ti_konsole(true, "konsole-256color");
#endif

    /*
    std::vector<std::string_view> vargv(argc-1);
    std::copy(argv + 1, argv + argc, vargv.begin());
    return http::testing(vargv);
    */
  } catch (std::exception &e) {
    std::println(stderr, "Exception: {}", e.what());
    return 1;
  }
}
