module;
#include <arpa/inet.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <print>
#include <vector>
export module terminfo;

export import :exception;

#ifndef UU
#define UU __attribute__((unused))
#endif

// RHE = read header element, a function
#define DECLARE_HE(e)                                                          \
  uint16_t e;                                                                  \
  std::string_view str_##e = #e
#define RHE_SHORTCUT(e) header.e, header.str_##e

namespace terminfo {

export enum class numerical_property_t {
  max_colors = 13,
};

export class reader {
private:
  struct header_t {
    DECLARE_HE(magic);
    DECLARE_HE(sz_terminal_names);
    DECLARE_HE(ni8_boolean_flags);
    DECLARE_HE(ni32_numbers);
    DECLARE_HE(ni16_strings);
    DECLARE_HE(sz_string_table);
  };

  enum class magic_t : uint16_t {
    legacy = 0432,           // = 0x011a = 282, when big-endian
    extended_number = 01036, // = 0x021e = 542
  };

  bool m_invert_endian = false;

  std::string m_terminal_names;
  std::vector<int8_t> m_boolean_flags;
  std::vector<int32_t> m_numbers;

public:
  reader() { load(); }
  reader(bool is_term_or_file, std::string_view str) {
    load(is_term_or_file, str);
  }

  auto const &get_terminal_names() const { return m_terminal_names; }
  auto const &get_boolean_flags() const { return m_boolean_flags; }
  auto const &get_numbers() const { return m_numbers; }

  auto get_numerical_property(numerical_property_t p) const -> int32_t {
    const auto pn = static_cast<size_t>(p);
    if (pn < m_numbers.size()) {
      return m_numbers[pn];
    } else {
      return -1;
    }
  }

private:
  // deduces from $TERM
  auto load() -> void {
    const char *term_env = getenv("TERM");
    if (!term_env)
      std::println(stderr, "Environment variable 'TERM' isn't defined. Can't "
                           "deduce terminal type");
    load(true, std::string(term_env));
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
    UU const auto header = _load_parse_header(file);

    // Step 4: Parse terminal names
    m_terminal_names.resize(header.sz_terminal_names);
    _load_read_generic(file, m_terminal_names.data(), m_terminal_names.size(),
                       false, "terminal names");
    if (!m_terminal_names.empty())
      m_terminal_names.resize(m_terminal_names.size() - 1);
    log(LT_INFO, "terminal names = {}", m_terminal_names);

    // Step 5: Parse boolean flags
    m_boolean_flags.resize(header.ni8_boolean_flags);
    _load_read_generic(file, m_boolean_flags.data(), m_boolean_flags.size(),
                       false, "boolean flags");

    // Step 5.1: Skip the padding byte
    if (header.ni8_boolean_flags % 2) {
      file.seekg(1, std::ios::cur);
    }

    // Step 6: Parse numbers
    m_numbers.resize(header.ni32_numbers);
    if (header.magic == static_cast<int>(magic_t::legacy)) {
      std::vector<int16_t> i16_numbers(header.ni32_numbers);
      _load_read_generic(file, i16_numbers.data(), i16_numbers.size() * 2,
                         false, "numbers");
      std::copy(i16_numbers.begin(), i16_numbers.end(), m_numbers.begin());
    } else {
      _load_read_generic(file, m_numbers.data(), m_numbers.size() * 4, false,
                         "numbers");
    }
    if (m_invert_endian) {
      std::ranges::for_each(m_numbers, [](int32_t &e) { return ntohl(e); });
    }
  }

  auto _load_read_generic(std::ifstream &file, void *into, size_t sz,
                          bool needs_endian_inversion,
                          std::string_view error_desc) -> void {
    file.read(reinterpret_cast<char *>(into), sz);
    if (!file)
      throw terminfo::exception(std::format("Could'nt extract {}", error_desc));
    if (needs_endian_inversion && m_invert_endian) {
      if (sz == 2) {
        auto &x = *reinterpret_cast<uint16_t *>(into);
        x = ntohs(x);
      } else if (sz == 4) {
        auto &x = *reinterpret_cast<uint32_t *>(into);
        x = ntohl(x);
      } else {
        throw terminfo::exception(
            std::format("Endian inversion is not available for {} byte long "
                        "values. Couldn't extract {}",
                        sz, error_desc));
      }
    }
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
      _load_read_generic(file, &elem, 2, true, str_elem);
      log(LT_INFO, "{} = {}", str_elem, elem);
    };

    const auto magic_one = static_cast<uint16_t>(magic_t::legacy);
    const auto magic_two = static_cast<uint16_t>(magic_t::extended_number);

    read_header_element(RHE_SHORTCUT(magic));
    if (!(header.magic == magic_one || header.magic == magic_two)) {
      const auto rev_one = ntohs(magic_one);
      const auto rev_two = ntohs(magic_two);
      if (header.magic == rev_one || header.magic == rev_two) {
        m_invert_endian = true;
        log(LT_INFO, "Enabling inverted endian mode");
      } else
        throw terminfo::exception(
            std::format("Unknown magic 0{:o}", header.magic));
    }

    read_header_element(RHE_SHORTCUT(sz_terminal_names));
    read_header_element(RHE_SHORTCUT(ni8_boolean_flags));
    read_header_element(RHE_SHORTCUT(ni32_numbers));
    read_header_element(RHE_SHORTCUT(ni16_strings));
    read_header_element(RHE_SHORTCUT(sz_string_table));

    return header;
  };

private:
  enum log_type_t { LT_INFO, LT_COUNT };
  static constexpr std::array<std::string_view, LT_COUNT> log_type_stringify{
      "INFO"};

  template <typename... Args>
  auto log(log_type_t type, std::format_string<Args...> fmt, Args &&...args)
      -> void {
    // NOTE: supressing annoying logs
    if (type == LT_INFO) return;
    const auto msg = std::format(fmt, std::forward<Args>(args)...);
    std::println(stderr, "{}: terminfo: {}",
                 log_type_stringify[static_cast<int>(type)], msg);
  }
};

} // namespace terminfo
