#include <arpa/inet.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <exception>
#include <print>
#include <ranges>
#include <vector>

import http;

auto to_string_poll_bitmask(uint16_t bits) {
  std::ostringstream repr;

#define _ELEM(x) std::make_pair<uint16_t, std::string_view>(x, #x)
  static constexpr std::array map{
      _ELEM(POLLIN),  _ELEM(POLLRDNORM), _ELEM(POLLRDBAND), _ELEM(POLLPRI),
      _ELEM(POLLOUT), _ELEM(POLLWRNORM), _ELEM(POLLWRBAND), _ELEM(POLLERR),
      _ELEM(POLLHUP), _ELEM(POLLNVAL)};
#undef _ELEM

  for (auto [mask, str] : map) {
    if (mask & bits) {
      if (!repr.view().empty())
        repr << " | ";
      repr << str;
    }
  }

  return repr.str();
}

auto testing(std::vector<std::string_view> const &argv) -> int {
  std::string address{"192.168.60.1"}, data_out{"GET / HTTP/1.1\n\n"};
  if (argv.size() > 0)
    address = argv[0];
  if (argv.size() > 1)
    data_out = argv[1];

  http::tcp_client c1(address.c_str(), 80);
  const auto fd = c1.get_fd();

  c1.connect();

  int status = 0;

  status = send(fd, data_out.c_str(), data_out.size(), 0);
  std::println("send(): written {}/{} bytes", status, data_out.size());

  std::string data_in;

  try {
    while (true) {
      // Check if anything is available to read
      pollfd fds[1]{};
      fds[0].fd = fd;
      fds[0].events = POLLIN;

      std::println("\nevents: {}", to_string_poll_bitmask(fds[0].events));
      status = poll(fds, 1, -1);
      std::println("poll(): revents: {}",
                   to_string_poll_bitmask(fds[0].revents));

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
    std::println("\nReceived:\n{}", data_in);
    throw;
  }

  return 0;
}

auto main(int argc, char **argv) -> int {
  try {
    std::vector<std::string_view> vargv;
    for (int i : std::views::iota(1, argc))
      vargv.push_back(argv[i]);
    return testing(vargv);
  } catch (std::exception &e) {
    std::println(stderr, "Exception: {}", e.what());
    return 1;
  }
}
