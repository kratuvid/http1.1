module;
#include <arpa/inet.h>
#include <sys/socket.h>

#include <cerrno>
#include <cstring>
#include <print>

#include "defines.hpp"
export module http;

export import :log;
export import :socket;
export import :util;

namespace http {

export auto testing() -> int {
  /*
  const auto sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
    QUIT_ERRNO_THROWING("Socket creation failed");

  sockaddr_in addr{};
  addr.sin_port = htons(80);
  addr.sin_family = AF_INET;
  if (inet_pton(addr.sin_family, "23.215.0.13", &addr.sin_addr) != 1)
    QUIT_THROWING("Couldn't convert IP from string to binary");

  if (connect(sock, (sockaddr *)&addr, sizeof(addr)) != 0)
    QUIT_ERRNO_THROWING("Couldn't connect to name");
   */

  csocket sock(AF_INET, SOCK_STREAM);

  HTTP_LOG_INFO("Hi, how are you?");
  HTTP_LOG_WARN("Wo ho ho, wait up buddy");
  HTTP_LOG_ERROR("You're at the point of no return");
  HTTP_LOG_FATAL("Goodbye!");

  return 0;
}

} // namespace http
