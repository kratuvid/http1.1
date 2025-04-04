module;
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <bit>
#include <cstring>
#include <expected>
#include <format>
#include <type_traits>

#include "defines.hpp"
export module http:socket;

import logger;
import :exception;

namespace http {

template <int Domain, int Type, bool IsServer>
  requires((Domain == AF_INET || Domain == AF_INET6) &&
           (Type == SOCK_STREAM || Type == SOCK_DGRAM))
class csocket {
private:
  static constexpr bool is_domain_ipv4 = Domain == AF_INET;
  static constexpr bool is_type_tcp = Type == SOCK_STREAM;
  using sockaddr_inX =
      std::conditional_t<is_domain_ipv4, sockaddr_in, sockaddr_in6>;
  using inX_addr = std::conditional_t<is_domain_ipv4, in_addr, in6_addr>;

public:
  /* Construction and destruction */
  csocket() : m_instance_index(m_instance_counter++) {
    static_assert(std::endian::native == std::endian::little ||
                      std::endian::native == std::endian::big,
                  "Mixed endian systems are unsupported");

    m_sockfd = socket(Domain, Type, 0);
    if (m_sockfd == -1)
      HTTP_LOG_FATAL_ERRNO("{}: Socket file descriptor creation failed",
                           _log_prefix_early());

    if constexpr (is_domain_ipv4) {
      this->m_sockaddr.sin_family = AF_INET;
    } else {
      this->m_sockaddr.sin6_family = AF_INET6;
    }

    HTTP_LOG_INFO("{}: csocket created [fd: {}]", _log_prefix_early(),
                  m_sockfd);
  }
  csocket(const char *address, uint16_t port) : csocket() {
    set_address(address);
    set_port(port);
  }
  csocket(inX_addr const &address, uint16_t port) : csocket() {
    set_address(address);
    set_port(port);
  }

  csocket(csocket const &) = delete;
  csocket(csocket &&other) { this->operator=(std::move(other)); }

  csocket &operator=(csocket const &) = delete;
  csocket &operator=(csocket &&other) {
    if (this != &other) {
      m_sockfd = other.m_sockfd;
      this->m_sockaddr = other.m_sockaddr;
      m_instance_index = other.m_instance_index;

      other.m_sockfd = 0;
      std::memset(&this->m_sockaddr, 0, sizeof(this->m_sockaddr));
      other.m_instance_index = -1;
    }

    HTTP_LOG_INFO("{}: csocket moved [fd: {}]", _log_prefix(), m_sockfd);

    return *this;
  }

  ~csocket() {
    if (m_sockfd > 0) {
      if (close(m_sockfd) != 0)
        HTTP_LOG_ERROR_ERRNO(
            "{}: Couldn't close the socket file descriptor '{}'",
            _log_prefix_early(), m_sockfd);
    }

    HTTP_LOG_INFO("{}: csocket destroyed [fd: {}]", _log_prefix(), m_sockfd);
  }

  /* Connection setup */
  auto bind(bool throw_on_failure = true)
    requires(IsServer)
  {
    static constexpr std::format_string<std::string> error_format{
        "{}: Couldn't bind the socket"};

    const auto status =
        ::bind(m_sockfd, reinterpret_cast<const sockaddr *>(&m_sockaddr),
               sizeof(m_sockaddr));

    if (status == -1) {
      if (throw_on_failure)
        HTTP_LOG_FATAL_ERRNO(error_format, _log_prefix());
      else
        HTTP_LOG_WARN_ERRNO(error_format, _log_prefix());
    } else {
      HTTP_LOG_INFO("{}: Bound", _log_prefix());
    }

    return status;
  }

  auto listen(int backlog, bool throw_on_failure = true)
    requires(IsServer && is_type_tcp)
  {
    static constexpr std::format_string<std::string> error_format{
        "{}: Couldn't begin listen on the socket"};

    const auto status = ::listen(m_sockfd, backlog);

    if (status == -1) {
      if (throw_on_failure)
        HTTP_LOG_FATAL_ERRNO(error_format, _log_prefix());
      else
        HTTP_LOG_WARN_ERRNO(error_format, _log_prefix());
    } else {
      HTTP_LOG_INFO("{}: Listening", _log_prefix());
    }

    return status;
  }

  auto accept(bool throw_on_failure = true)
    requires(IsServer && is_type_tcp)
  {
    static constexpr std::format_string<std::string> error_format{
        "{}: Couldn't accept on the socket"};

    HTTP_LOG_INFO("{}: Accepting...", _log_prefix());

    sockaddr_inX client_sockaddr{};
    socklen_t client_socklen = sizeof(sockaddr_inX);

    const auto status =
        ::accept(m_sockfd, reinterpret_cast<sockaddr *>(&client_sockaddr),
                 &client_socklen);

    if (status == -1) {
      if (throw_on_failure)
        HTTP_LOG_FATAL_ERRNO(error_format, _log_prefix());
      else
        HTTP_LOG_WARN_ERRNO(error_format, _log_prefix());
    } else {
      HTTP_LOG_INFO("{}: Accepted a new connection [fd: {}, client: {}:{}]",
                    _log_prefix(), status, _get_sinX_addr_any(client_sockaddr),
                    _get_sinX_port_any(client_sockaddr));
    }

    struct {
      int status;
      sockaddr_inX sockaddr;
      socklen_t socklen;
    } out{status, client_sockaddr, client_socklen};
    return out;
  }

  auto connect(bool throw_on_failure = true)
    requires(!IsServer)
  {
    static constexpr std::format_string<std::string> error_format{
        "{}: Connection failed"};

    HTTP_LOG_INFO("{}: Connecting...", _log_prefix());

    const auto status =
        ::connect(m_sockfd, reinterpret_cast<sockaddr *>(&m_sockaddr),
                  sizeof(m_sockaddr));

    if (status == -1) {
      if (throw_on_failure)
        HTTP_LOG_FATAL_ERRNO(error_format, _log_prefix());
      else
        HTTP_LOG_WARN_ERRNO(error_format, _log_prefix());
    } else {
      HTTP_LOG_INFO("{}: Connected", _log_prefix());
    }

    return status;
  }

  /* Communication */
  // TODO

  /* Basic setters and getters */
  auto set_address(const char *address_str) -> void {
    if (!address_str)
      HTTP_LOG_FATAL("{}: Address string can't be null", _log_prefix());

    const auto status = inet_pton(Domain, address_str, &_get_sinX_addr());

    if (status == 0) {
      HTTP_LOG_FATAL("{}: Invalid IPv{} address: {}", _log_prefix(),
                     _get_domain_str(), address_str);
    } else if (status == -1) {
      HTTP_LOG_FATAL("{}: Invalid address family: {}", _log_prefix(), Domain);
    } else {
      // HTTP_LOG_INFO("{}: Address changed", _log_prefix());
    }
  }

  /*
  auto set_address(inX_addr address) -> void {
    if constexpr (std::endian::native == std::endian::little) {
      if constexpr (Domain == AF_INET) {
        address.s_addr = htonl(address.s_addr);
      } else {
        auto it_as_bytes = reinterpret_cast<std::byte *>(&address);
        std::reverse(it_as_bytes, it_as_bytes + 16);
      }
    }
    set_address_big_endian(address);
    }
  */

  auto set_address(inX_addr const &address) -> void {
    _get_sinX_addr() = address;
    // std::memcpy(&_get_sinX_addr(), &address, sizeof(address));
    // HTTP_LOG_INFO("{}: Address changed", _log_prefix());
  }

  // auto set_port_natural(uint16_t port) -> void { set_port(htons(port)); }
  auto set_port(uint16_t port) -> void {
    _get_sinX_port() = port;
    // HTTP_LOG_INFO("{}: Port changed", _log_prefix());
  }

  auto get_address() const { return _get_sinX_addr(); };
  auto get_address_str() const { return std::format("{}", _get_sinX_addr()); }

  auto get_port_hostorder() const {
    const uint16_t x = _get_sinX_port();
    return ntohs(x);
  }

  auto get_fd() const { return m_sockfd; };
  auto get_sockaddr() const { return this->m_sockaddr; }
  auto get_sockaddr_size() const { return this->m_sockaddr; }

private:
  const auto &_get_sinX_addr() const {
    if constexpr (is_domain_ipv4) {
      return this->m_sockaddr.sin_addr;
    } else {
      return this->m_sockaddr.sin6_addr;
    }
  }

  auto &_get_sinX_addr() {
    /*
    using clvalue_t = decltype(std::declval<const csocket>()._get_sinX_addr());
    using decayed_t = std::decay_t<clvalue_t>;
    using lvalue_t = std::add_lvalue_reference_t<decayed_t>;
    */
    return const_cast<inX_addr &>(
        static_cast<const csocket *>(this)->_get_sinX_addr());
  }

  auto _get_sinX_port() const -> const uint16_t & {
    if constexpr (is_domain_ipv4) {
      static_assert(
          std::is_same_v<decltype(this->m_sockaddr.sin_port), uint16_t>);
      return this->m_sockaddr.sin_port;
    } else {
      static_assert(
          std::is_same_v<decltype(this->m_sockaddr.sin6_port), uint16_t>);
      return this->m_sockaddr.sin6_port;
    }
  }

  auto &_get_sinX_port() {
    return const_cast<uint16_t &>(
        static_cast<const csocket *>(this)->_get_sinX_port());
  }

  auto _get_sinX_addr_offset() const -> size_t {
    return reinterpret_cast<const std::byte *>(&_get_sinX_addr()) -
           reinterpret_cast<const std::byte *>(&m_sockaddr);
  }

  auto _get_sinX_port_offset() const -> size_t {
    return reinterpret_cast<const std::byte *>(&_get_sinX_port()) -
           reinterpret_cast<const std::byte *>(&m_sockaddr);
  }

  auto _get_sinX_addr_any(sockaddr_inX const &sa) const -> inX_addr const & {
    if constexpr (is_domain_ipv4)
      return sa.sin_addr;
    else
      return sa.sin6_addr;
  }

  auto _get_sinX_port_any(sockaddr_inX const &sa) const -> uint16_t const & {
    if constexpr (is_domain_ipv4)
      return sa.sin_port;
    else
      return sa.sin6_port;
  }

  auto _get_domain_str() const -> std::string_view {
    using namespace std::literals;
    if constexpr (is_domain_ipv4)
      return "4"sv;
    return "6"sv;
  }

  auto _get_type_str() const -> std::string_view {
    using namespace std::literals;
    if constexpr (is_type_tcp)
      return "tcp"sv;
    return "udp"sv;
  }

  auto _get_mode_str() const -> std::string_view {
    using namespace std::literals;
    if constexpr (IsServer)
      return "server"sv;
    return "client"sv;
  }

  auto _log_prefix_basic() const -> std::string {
    return std::format("csocket(<{}{}{}> #{})", _get_type_str()[0],
                       _get_domain_str()[0], _get_mode_str()[0],
                       m_instance_index);
  }

  auto _log_prefix_early() const -> std::string {
    return std::format("{}", _log_prefix_basic());
  }

  auto _log_prefix() const -> std::string {
    return std::format("{}: {}:{}", _log_prefix_basic(), get_address_str(),
                       get_port_hostorder());
  }

private:
  int m_sockfd;
  sockaddr_inX m_sockaddr;

  int m_instance_index;
  inline static std::atomic_int m_instance_counter = 0;
};

export {
  using tcp_server = csocket<AF_INET, SOCK_STREAM, true>;
  using tcp6_server = csocket<AF_INET6, SOCK_STREAM, true>;
  using udp_server = csocket<AF_INET, SOCK_DGRAM, true>;
  using udp6_server = csocket<AF_INET6, SOCK_DGRAM, true>;

  using tcp_client = csocket<AF_INET, SOCK_STREAM, false>;
  using tcp6_client = csocket<AF_INET6, SOCK_STREAM, false>;
  using udp_client = csocket<AF_INET, SOCK_DGRAM, false>;
  using udp6_client = csocket<AF_INET6, SOCK_DGRAM, false>;
}

} // namespace http

// std::formatter specializations
template <typename ParseContext>
constexpr auto _parse(char identifier, ParseContext &ctx) {
  auto it = ctx.begin();
  if (it != ctx.end() && *it != '}')
    std::unexpected{
        std::format("Invalid format args for IPv{} inX_addr", identifier)};
  return std::expected<decltype(it), std::string>{it};
}

template <size_t BufferSize, int Domain, typename FmtContext>
auto _format(auto &&obj, FmtContext &ctx) {
  std::string formatted(BufferSize, '\0');
  inet_ntop(Domain, &obj, formatted.data(), formatted.size());
  formatted.resize(std::strlen(formatted.c_str()));
  return std::ranges::copy(std::move(formatted), ctx.out()).out;
}

export template <> struct std::formatter<in_addr, char> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    auto result = _parse('4', ctx);
    if (!result)
      throw std::format_error(std::move(result.error()));
    return result.value();
  }

  template <typename FmtContext>
  auto format(auto &&obj, FmtContext &ctx) const {
    return _format<INET_ADDRSTRLEN, AF_INET>(obj, ctx);
  }
};

export template <> struct std::formatter<in6_addr, char> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    auto result = _parse('6', ctx);
    if (!result)
      throw std::format_error(std::move(result.error()));
    return result.value();
  }

  template <typename FmtContext>
  auto format(auto &&obj, FmtContext &ctx) const {
    return _format<INET6_ADDRSTRLEN, AF_INET6>(obj, ctx);
  }
};
