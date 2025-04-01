module;
#include <cstdlib>

#include <filesystem>
#include <fstream>
#include <print>
#include <vector>
#include <unordered_map>
export module terminfo;

export import :exception;

#ifdef UU
#undef UU
#endif
#define UU __attribute__((unused))

#define DEFINE_HEADER_ELEM(e)                                                  \
  uint16_t e;                                                                  \
  std::string_view str_##e = #e

#define RHE_SHORTCUT(e) header.e, header.str_##e

namespace terminfo {

export class reader {
private:
  struct header_t {
    DEFINE_HEADER_ELEM(magic);
    DEFINE_HEADER_ELEM(sz_terminal_names);
    DEFINE_HEADER_ELEM(nbytes_boolean_flags);
    DEFINE_HEADER_ELEM(n16or32_numbers);
    DEFINE_HEADER_ELEM(nshorts_strings);
    DEFINE_HEADER_ELEM(sz_string_table);
  };

  enum class magic_t : uint16_t {
    legacy = 0432, // 0x011a, when big-endian
    extended_number = 01036, // 0x021e
  };

  std::string m_terminal_names;
  std::vector<int8_t> m_boolean_flags;
  std::variant<std::vector<int16_t>, std::vector<int32_t>> m_numbers;

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
      std::println(stderr, "Environment variable 'TERM' isn't defined. Can't "
                           "deduce terminal type");
    load(true, term_env);
  }

  // str is either a TERM name or a path to a terminfo file
  auto load(bool is_term_or_file, std::string_view str) -> void {
    if (str.empty()) {
      throw terminfo::exception("No string value provided to "
                                "terminfo::reader::load(). Is $TERM empty?");
    }

    // Step 1: Deduce the path to the particular terminfo file
    const auto filepath = _load_deduce_path(is_term_or_file, str);

    // Step 2: Open the file
    std::ifstream file(filepath, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
      throw terminfo::exception(
          std::format("Couldn't open terminfo file {}", filepath.native()));
    }

    // Step 3: Parse the header
    const auto header = _load_parse_header(file);

    // Step 4: Parse terminal names
    m_terminal_names.resize(header.sz_terminal_names);
    _load_read_generic(file, m_terminal_names.data(), m_terminal_names.size(),
                       "terminal names");
    if (!m_terminal_names.empty())
      m_terminal_names.resize(m_terminal_names.size() - 1);
    log(LT_INFO, "terminal names = {}", m_terminal_names);

    // Step 5: Parse boolean flags
    m_boolean_flags.resize(header.nbytes_boolean_flags);
    _load_read_generic(file, m_boolean_flags.data(), m_boolean_flags.size(), "boolean flags");
    
    // Step 5.1: Skip the padding byte
    if (header.nbytes_boolean_flags % 2) {
      file.seekg(1, std::ios::cur);
    }

    // Step 6: Parse numbers
    // TODO
  }

  auto _load_read_generic(std::ifstream &file, void *into, size_t sz,
                          std::string_view error_desc) -> void {
    file.read(reinterpret_cast<char *>(into), sz);
    
    if (!file)
      throw terminfo::exception(std::format("Could'nt extract {}", error_desc));
  }

  auto _load_deduce_path(bool is_term_or_file, std::string_view str)
      -> std::filesystem::path {
    std::filesystem::path filepath;

    auto f_existence = [this, &filepath]() -> bool {
      if (is_regular_file(filepath)) {
        this->log(LT_INFO, "{} is the chosen terminfo file", filepath.native());
        return true;
      } else {
        filepath.clear();
        return false;
      }
    };

    if (is_term_or_file) {
      const char *home_env = getenv("HOME");
      if (home_env) {
        filepath = std::format("{}/.terminfo/{}/{}", home_env, str.at(0), str);
        f_existence();
      }

      if (filepath.empty()) {
        filepath = std::format("/usr/share/terminfo/{}/{}", str.at(0), str);
      }
    } else {
      filepath = str;
    }

    if (!f_existence()) {
      throw terminfo::exception(
          "Terminal type couldn't be deduced from any heuristic");
    }

    return filepath;
  }

  auto _load_parse_header(std::ifstream &file) -> header_t {
    header_t header{};

    auto read_header_element =
        [this, &file](uint16_t &elem, std::string_view str_elem) -> void {
      _load_read_generic(file, &elem, 2, str_elem);
      log(LT_INFO, "{} = {}", str_elem, elem);
    };

    read_header_element(RHE_SHORTCUT(magic));
    if (!(header.magic == static_cast<uint16_t>(magic_t::legacy) ||
          header.magic == static_cast<uint16_t>(magic_t::extended_number))) {
      throw terminfo::exception(
          std::format("Unknown magic 0{:o}", header.magic));
    }

    read_header_element(RHE_SHORTCUT(sz_terminal_names));
    read_header_element(RHE_SHORTCUT(nbytes_boolean_flags));
    read_header_element(RHE_SHORTCUT(n16or32_numbers));
    read_header_element(RHE_SHORTCUT(nshorts_strings));
    read_header_element(RHE_SHORTCUT(sz_string_table));

    return header;
  };

private:
  enum log_type {
    LT_INFO,
  };

  auto log_type_to_string(log_type type) -> std::string_view {
    switch (type) {
    case LT_INFO:
      return "INFO";
      break;
    }
  }

  template <typename... Args>
  auto log(log_type type, std::format_string<Args...> fmt, Args &&...args)
      -> void {
    const auto msg = std::format(fmt, std::forward<Args>(args)...);
    std::println(stderr, "{}: terminfo: {}", log_type_to_string(type), msg);
  }
};

} // namespace terminfo
