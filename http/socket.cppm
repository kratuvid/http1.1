module;
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <format>
#include <variant>

#include "defines.hpp"
export module http:socket;

import :exception;

namespace http {

class csocket {
public:
  csocket(int domain, int type) : m_instance_index(m_instance_counter++) {
    if (!(domain == AF_INET || domain == AF_INET6))
      QUIT_THROWING("{}: {}", _log_prefix_early(),
                    "Only AF_INET and AF_INET6 are supported");

    m_sockfd = socket(domain, type, 0);
    if (m_sockfd == -1)
      QUIT_ERRNO_THROWING("{}: Socket file descriptor creation failed",
                          _log_prefix_early());
    m_sockfd = 100;
  }
  csocket(csocket const &) = delete;
  csocket(csocket &&) = default;
  csocket &operator=(csocket const &) = delete;
  csocket &operator=(csocket &&) = default;

  ~csocket() {
    if (m_sockfd > 0) {
      if (close(m_sockfd) != 0)
        QUIT_ERRNO_THROWING("{}: Couldn't close file descriptor '{}'",
                            _log_prefix_early(), m_sockfd);
    }
  }

public:
  auto get_ascii_address() const {
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
      QUIT_ERRNO_THROWING(
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
    return std::format("#{} ({}:{})", m_instance_index, get_ascii_address(),
                       get_port());
  }

private:
  int m_sockfd;
  std::variant<sockaddr_in, sockaddr_in6> m_sockaddr{};

  inline static int m_instance_counter = 0;
  int m_instance_index;
};

} // namespace http
