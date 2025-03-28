module;
#include <arpa/inet.h>
#include <sys/socket.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <print>
#include <thread>

#include "defines.hpp"
export module http;

export import :log;
export import :socket;
export import :util;
export import :exception;

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

  /*
  csocket<AF_INET, SOCK_STREAM, false> sk;
  csocket<AF_INET6, SOCK_STREAM, false> sk6;

  std::println("sk:");
  std::println("size: {}", sizeof sk);
  std::println("address: {}", sk.get_address_str());
  std::println("port: {}", sk.get_port());

  std::println("sk6:");
  std::println("size: {}", sizeof sk6);
  std::println("address: {}", sk6.get_address_str());
  std::println("port: {}", sk6.get_port());
   */

  tcp_client c1("127.0.0.1", 8000);
  c1.connect();

  /*
  tcp6_server s1("::1", 8000);
  s1.bind();
  s1.listen(8);
  s1.accept();
  */

  return 0;
}

} // namespace http
