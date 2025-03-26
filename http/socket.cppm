module;
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <format>
#include <type_traits>

#include "defines.hpp"
export module http:socket;

import :log;
import :exception;

namespace http {

template <int Domain> class _domain_base {
protected:
  sockaddr_in m_sockaddr{};
};
template <> class _domain_base<AF_INET6> {
protected:
  sockaddr_in6 m_sockaddr{};
};

template <int Domain, int Type, bool IsServer>
  requires((Domain == AF_INET || Domain == AF_INET6) &&
           (Type == SOCK_STREAM || Type == SOCK_DGRAM))
class csocket : public _domain_base<Domain> {
public:
  csocket() : m_instance_index(m_instance_counter++) {
    m_sockfd = socket(Domain, Type, 0);
    if (m_sockfd == -1)
      HTTP_LOG_FATAL_ERRNO("{}: Socket file descriptor creation failed",
                           _log_prefix_early());

    if constexpr (Domain == AF_INET) {
      this->m_sockaddr.sin_family = AF_INET;
    } else {
      this->m_sockaddr.sin6_family = AF_INET6;
    }
  }

  csocket(csocket const &) = delete;
  csocket(csocket &&other) { this->operator=(std::move(other)); }

  csocket &operator=(csocket const &) = delete;
  csocket &operator=(csocket &&other) {
    m_sockfd = other.m_sockfd;
    this->m_sockaddr = other.m_sockaddr;
    m_instance_index = other.m_instance_index;

    other.m_sockfd = 0;
    std::memset(&this->m_sockaddr, 0, sizeof(this->m_sockaddr));
    other.m_instance_index = -1;

    return *this;
  }

  ~csocket() {
    if (m_sockfd > 0) {
      if (close(m_sockfd) != 0)
        HTTP_LOG_ERROR_ERRNO("{}: Couldn't close file descriptor '{}'",
                             _log_prefix_early(), m_sockfd);
    }
  }

public:
  auto set_address(const char *address_str) -> void {
    if (!address_str)
      HTTP_LOG_FATAL("{}: Address is null", _log_prefix());

    const auto status = inet_pton(Domain, address_str, &_get_sinX_addr());

    if (status == 0) {
      HTTP_LOG_FATAL("{}: {} isn't a valid IPv{} address", _log_prefix(),
                     address_str, is_af_ipv4() ? '4' : '6');
    } else if (status == -1) {
      HTTP_LOG_FATAL_ERRNO("{}: {} isn't a valid address family", _log_prefix(),
                           Domain);
    }
  }

  auto set_address(auto inX_addr) -> void {
    if constexpr (Domain == AF_INET) {
      inX_addr = htonl(inX_addr);
    } else {
      auto it_as_bytes = static_cast<std::byte *>(&inX_addr);
      std::reverse(it_as_bytes, it_as_bytes + 16);
    }
    set_address_be(inX_addr);
  }

  auto set_address_be(auto const &inX_addr) -> void
    requires(std::is_same_v<
             std::decay_t<decltype(inX_addr)>,
             std::decay_t<decltype(std::declval<csocket>()._get_sinX_addr())>>)
  /* requires((Domain == AF_INET &&
            std::is_same_v<in_addr, std::decay_t<decltype(inX_addr)>>) ||
           (Domain == AF_INET6 &&
            std::is_same_v<in6_addr, std::decay_t<decltype(inX_addr)>>)) */
  {
    std::memcpy(&_get_sinX_addr(), &inX_addr, sizeof(inX_addr));
    /* if constexpr (Domain == AF_INET) {
      std::memcpy(&this->m_sockaddr.sin_addr, &inX_addr, sizeof(inX_addr));
    } else {
      std::memcpy(&this->m_sockaddr.sin6_addr, &inX_addr, sizeof(inX_addr));
    } */
  }

  auto set_port(uint16_t port) -> void { set_port_be(htons(port)); }
  auto set_port_be(uint16_t port) -> void {
    _get_sinX_port() = port;
    /* if constexpr (Domain == AF_INET) {
      this->m_sockaddr.sin_port = port;
    } else {
      this->m_sockaddr.sin6_port = port;
    } */
  }

  auto get_address() const {
    return _get_sinX_addr();
    /* if constexpr (Domain == AF_INET) {
      return this->m_sockaddr.sin_addr;
    } else {
      return this->m_sockaddr.sin6_addr;
    } */
  };

  auto get_address_str() const {
    std::string address_out;
    if constexpr (Domain == AF_INET) {
      address_out.resize(INET_ADDRSTRLEN + 1);
    } else {
      address_out.resize(INET6_ADDRSTRLEN + 1);
    }

    const void *const p_sinX_addr = &_get_sinX_addr();
    /* if constexpr (Domain == AF_INET) {
      p_sinX_addr = &this->m_sockaddr.sin_addr;
    } else {
      p_sinX_addr = &this->m_sockaddr.sin6_addr;
    } */

    const auto out =
        inet_ntop(Domain, p_sinX_addr, address_out.data(), address_out.size());
    if (!out)
      HTTP_LOG_FATAL_ERRNO(
          "{}: Binary to ASCII conversion failed for the IP address",
          _log_prefix_early());

    address_out.resize(std::strlen(address_out.c_str()));

    return address_out;
  }

  auto get_port() const {
    return ntohs(_get_sinX_port());
    /* if constexpr (Domain == AF_INET) {
      return ntohs(this->m_sockaddr.sin_port);
    } else {
      return ntohs(this->m_sockaddr.sin6_port);
    } */
  }

  auto get_fd() const { return m_sockfd; };
  auto get_sockaddr() const { return this->m_sockaddr; }
  auto get_sockaddr_size() const { return this->m_sockaddr; }

  constexpr auto is_af_ipv4() const { return Domain == AF_INET; }

private:
  const auto &_get_sinX_addr() const {
    if constexpr (Domain == AF_INET) {
      return this->m_sockaddr.sin_addr;
    } else {
      return this->m_sockaddr.sin6_addr;
    }
  }

  auto &_get_sinX_addr() {
    using clvalue_t = decltype(std::declval<const csocket>()._get_sinX_addr());
    using decayed_t = std::decay_t<clvalue_t>;
    using lvalue_t = std::add_lvalue_reference_t<decayed_t>;
    return const_cast<lvalue_t>(
        static_cast<const csocket *>(this)->_get_sinX_addr());
  }

  const uint16_t &_get_sinX_port() const {
    if constexpr (Domain == AF_INET) {
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

  auto _log_prefix_early() const -> std::string {
    return std::format("#{}", m_instance_index);
  }

  auto _log_prefix() const -> std::string {
    return std::format("#{} ({}:{})", m_instance_index, get_address_str(),
                       get_port());
  }

private:
  int m_sockfd;
  int m_instance_index;

  inline static int m_instance_counter = 0;
};

using tcp_server = csocket<AF_INET, SOCK_STREAM, true>;
using tcp6_server = csocket<AF_INET6, SOCK_STREAM, true>;
using udp_server = csocket<AF_INET, SOCK_DGRAM, true>;
using udp6_server = csocket<AF_INET6, SOCK_DGRAM, true>;

using tcp_client = csocket<AF_INET, SOCK_STREAM, false>;
using tcp6_client = csocket<AF_INET6, SOCK_STREAM, false>;
using udp_client = csocket<AF_INET, SOCK_DGRAM, false>;
using udp6_client = csocket<AF_INET6, SOCK_DGRAM, false>;

} // namespace http
