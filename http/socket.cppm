module;
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <format>

#include "defines.hpp"
export module http:socket;

import :log;
import :exception;

namespace http {

template <int Domain> class _domain_base {
public:
  sockaddr_in m_sockaddr{};
};
template <> class _domain_base<AF_INET6> {
public:
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
  auto get_address_str() const {
    // Max bytes it takes to represent an ASCII IPv6 address
    std::string address_out(16 * 2 + 7 + 1, '\0');

    const auto out = inet_ntop(Domain, this->m_sockaddr, address_out.data(),
                               address_out.size());
    if (!out)
      HTTP_LOG_FATAL_ERRNO(
          "{}: Binary to ASCII conversion failed for the IP address",
          _log_prefix_early());

    return address_out;
  }

  auto get_port() const {
    if constexpr (Domain == AF_INET) {
      return this->m_sockaddr.sin_port;
    } else {
      return this->m_sockaddr.sin6_port;
    }
  }

  auto get_fd() const { return m_sockfd; };
  auto get_sockaddr() const { return this->m_sockaddr; }
  auto get_sockaddr_size() const { return this->m_sockaddr; }

  constexpr auto is_ipv4() const { return Domain == AF_INET; }

private:
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
