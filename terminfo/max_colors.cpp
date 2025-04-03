#include <exception>
#include <print>
#include <ranges>

import terminfo;

auto main(int argc, char **argv) -> int {
  try {
    terminfo::reader::m_suppress_logs = false;
    if (argc < 2) {
      terminfo::reader ti;
      std::println();
      terminfo::reader ti_legacy(true, "xterm");
      std::println();
      terminfo::reader ti_konsole(true, "konsole-256color");
      auto *pti = &ti;
      auto *pti_legacy = &ti_legacy;
      auto *pti_konsole = &ti_konsole;

      std::println();
      std::println("max_colors: $TERM = {}, xterm = {}, konsole = {}",
                   pti->get_numerical_property(
                       terminfo::numerical_property_t::max_colors),
                   pti_legacy->get_numerical_property(
                       terminfo::numerical_property_t::max_colors),
                   pti_konsole->get_numerical_property(
                       terminfo::numerical_property_t::max_colors));
    } else {
      std::vector<int32_t> v_max_colors(argc - 1);
      for (int i : std::views::iota(1, argc)) {
        const terminfo::reader ti_custom(true, argv[i]);
        v_max_colors[i - 1] = ti_custom.get_numerical_property(
            terminfo::numerical_property_t::max_colors);
        std::println();
      }

      std::println("max_colors:");
      for (int i : std::views::iota(1, argc)) {
        std::println("{}: {}", argv[i], v_max_colors[i - 1]);
      }
    }
  } catch (std::exception &e) {
    std::println(stderr, "Exception: {}", e.what());
    return 1;
  }
}
