#include <arpa/inet.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <exception>
#include <print>
#include <string>
#include <vector>
#include <ranges>

#include "defines.hpp"

import logger;
import http;

auto to_string_poll_events(uint16_t bits) {
#define _ELEM(x) std::make_pair<uint16_t, std::string_view>(x, #x)
  static constexpr std::array map{
      _ELEM(POLLIN),  _ELEM(POLLRDNORM), _ELEM(POLLRDBAND), _ELEM(POLLPRI),
      _ELEM(POLLOUT), _ELEM(POLLWRNORM), _ELEM(POLLWRBAND), _ELEM(POLLERR),
      _ELEM(POLLHUP), _ELEM(POLLNVAL)};
#undef _ELEM

  std::string repr;
  
  for (auto [mask, str] : map) {
    if (mask & bits) {
      if (!repr.empty())
        repr += " | ";
      repr += str;
    }
  }

  return repr.empty() ? "0" : repr;
}

auto testing(std::vector<std::string_view> const &argv) -> int {
  int status = 0;

  std::string hostname{"google.com"};
  std::string data_out{"GET / HTTP/1.1\r\n"};
  int port = 80;

  if (argv.size() >= 1)
    hostname = argv[0];
  if (argv.size() >= 2)
    port = std::atoi(argv[1].data());
  if (argv.size() >= 3)
    data_out.clear();

  for (size_t i = 2; i < argv.size(); i++) {
    data_out += std::format("{}\r\n", argv[i]);
  }
  data_out += "Host: " + hostname + "\r\n";
  data_out += "Connection: close\r\n";
  data_out += "\r\n";

  addrinfo *ai{};

  addrinfo hints{};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  status = getaddrinfo(hostname.c_str(), nullptr, &hints, &ai);

  if (status != 0)
    HTTP_LOG_FATAL("getaddrinfo failed: {}", gai_strerror(status));

  const auto addr_raw = reinterpret_cast<sockaddr_in*>(ai->ai_addr)->sin_addr;

  freeaddrinfo(ai);

  http::tcp_client c1(addr_raw, htons(port));
  const auto fd = c1.get_fd();

  c1.connect();

  status = send(fd, data_out.c_str(), data_out.size(), 0);
  std::println("send(): written {}/{} bytes", status, data_out.size());

  std::string data_in;

  try {
    while (true) {
      // Check if anything is available to read
      pollfd fds[1]{};
      fds[0].fd = fd;
      fds[0].events = POLLIN;

      std::println("\nevents: {}", to_string_poll_events(fds[0].events));
      status = poll(fds, 1, -1);
      std::println("poll(): revents: {}",
                   to_string_poll_events(fds[0].revents));

      if (status > 0)
        std::println("poll(): {} events selected", status);
      else if (status == 0)
        throw std::runtime_error(std::format("poll(): timed-out"));
      else if (status == -1)
        throw std::runtime_error(
            std::format("poll(): error: {}", std::strerror(errno)));

      if (fds[0].revents & POLLHUP)
        throw std::runtime_error(std::format("poll(): POLLHUP"));
      else if (fds[0].revents & POLLERR)
        throw std::runtime_error(std::format("poll(): POLLERR"));

      // How much?
      int bytes_available{};
      if (ioctl(fd, FIONREAD, &bytes_available) == -1)
        throw std::runtime_error(
            std::format("ioctl(): error: {}", std::strerror(errno)));
      std::println("ioctl(): {} bytes available", bytes_available);

      // Read it
      auto prev_size = data_in.size();
      data_in.resize(data_in.size() + bytes_available);
      status = recv(fd, data_in.data() + prev_size, bytes_available, 0);
      if (status == -1)
        throw std::runtime_error(
            std::format("recv(): error: {}", std::strerror(errno)));
      else if (status == 0)
        throw std::runtime_error(std::format("recv(): peer has shut down"));
      std::println("recv(): read {}/{} bytes", status, bytes_available);

      // std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  } catch (...) {
    std::println("\nReceived ({} bytes):\n{}", data_in.size(), data_in);
    throw;
  }

  return 0;
}

auto main(int argc, char **argv) -> int {
  try {
    std::vector<std::string_view> vargv(argc-1);
    std::copy(argv + 1, argv + argc, vargv.begin());
    return testing(vargv);
  } catch (std::exception &e) {
    std::println(stderr, "Exception: {}", e.what());
    return 1;
  }
}
