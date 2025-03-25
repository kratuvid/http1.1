module;
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <format>
#include <variant>

#include "defines.hpp"
export module http:socket;

import :log;
import :exception;

namespace http {

class csocket {
public:
  csocket(int domain, int type) : m_instance_index(m_instance_counter++) {
    if (!(domain == AF_INET || domain == AF_INET6))
      HTTP_LOG_FATAL("{}: {}", _log_prefix_early(),
                    "Only AF_INET and AF_INET6 domains are supported");

    m_sockfd = socket(domain, type, 0);
    if (m_sockfd == -1)
      HTTP_LOG_FATAL_ERRNO("{}: Socket file descriptor creation failed",
                          _log_prefix_early());

    switch (domain) {
    case AF_INET: {
      // .._in is the default. just pull and modify it
      auto &sa = std::get<sockaddr_in>(m_sockaddr);
      sa.sin_family = AF_INET;
    } break;
    case AF_INET6: {
      sockaddr_in6 sa {};
      sa.sin6_family = AF_INET6;
      m_sockaddr = sa;
    } break;
    }
  }

  csocket(csocket const &) = delete;
  csocket(csocket &&other) { this->operator=(std::move(other)); }

  csocket &operator=(csocket const &) = delete;
  csocket &operator=(csocket &&other) {
    m_sockfd = other.m_sockfd;
    m_sockaddr = std::move(other.m_sockaddr);
    m_instance_index = other.m_instance_index;
    other.m_sockfd = 0;
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
    std::string addr_buf(16 * 2 + 8 + 1, '\0');

    const void *ptr_sockaddr_in = nullptr;
    std::visit(
        [&ptr_sockaddr_in](auto &&value) {
          ptr_sockaddr_in = static_cast<const void *>(&value);
        },
        m_sockaddr);

    auto buf =
        inet_ntop(_get_af(), ptr_sockaddr_in, addr_buf.data(), addr_buf.size());
    if (!buf)
      HTTP_LOG_FATAL_ERRNO(
          "{}: Binary to ASCII conversion failed for the IP address",
          _log_prefix_early());

    return addr_buf;
  }

  auto get_port() const {
    if (is_inet4())
      return std::get<sockaddr_in>(m_sockaddr).sin_port;
    else
      return std::get<sockaddr_in6>(m_sockaddr).sin6_port;
  }

  auto get_fd() const { return m_sockfd; };

  auto get_sockaddr() const { return m_sockaddr; }
  auto get_sockaddr_size() const {
    return is_inet4() ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
  }

  constexpr bool is_inet4() const {
    return std::holds_alternative<sockaddr_in>(m_sockaddr);
  }

private:
  auto _get_af() const -> int { return is_inet4() ? AF_INET : AF_INET6; };

  auto _log_prefix_early() const -> std::string {
    return std::format("#{}", m_instance_index);
  }

  auto _log_prefix() const -> std::string {
    return std::format("#{} ({}:{})", m_instance_index, get_address_str(),
                       get_port());
  }

private:
  int m_sockfd;
  std::variant<sockaddr_in, sockaddr_in6> m_sockaddr{};

  inline static int m_instance_counter = 0;
  int m_instance_index;
};

} // namespace http
