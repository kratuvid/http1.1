module;
#include <cstdlib>

#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "defines.hpp"
export module terminfo;

import logger;

export import :exception;

namespace terminfo {

class reader {
public:
  reader() { load(); }
  reader(bool is_term_or_file, std::string_view str) {
    load(is_term_or_file, str);
  }

private:
  // deduces from $TERM
  auto load() -> void {
    const char *term_env = getenv("TERM");
    if (!term_env)
      TERMINFO_LOG_FATAL("Environment variable 'TERM' isn't defined. Can't "
                         "deduce terminal type");
    load(true, term_env);
  }

  // str is either a TERM name or a path to a terminfo file
  auto load(bool is_term_or_file, std::string_view str) -> void {
    std::filesystem::path filepath;
    if (is_term_or_file) {
      const char *home_env = getenv("HOME");
      if (home_env) {
        filepath = std::format("{}/{}/{}", home_env, str.at(0), str);

        std::error_code ec;
        if (is_regular_file(filepath, ec)) {
          TERMINFO_LOG_INFO("{} is the chosen terminfo file", filepath.native());
        } else {
          filepath.clear();
        }
      }

      if (filepath.empty()) {
        filepath = std::format("/usr/share/terminfo/{}/{}", str.at(0), str);
      }

    } else {
      filepath = str;
    }
  }
};

} // namespace terminfo
