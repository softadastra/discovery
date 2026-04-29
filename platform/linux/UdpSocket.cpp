/**
 *
 *  @file UdpSocket.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *
 *  Softadastra Discovery
 *
 */

#include <softadastra/discovery/platform/linux/UdpSocket.hpp>

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace softadastra::discovery::platform::os_linux
{

  namespace
  {

    [[nodiscard]] bool make_sockaddr(
        const std::string &host,
        std::uint16_t port,
        ::sockaddr_in &addr) noexcept
    {
      addr = {};
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);

      return ::inet_pton(
                 AF_INET,
                 host.c_str(),
                 &addr.sin_addr) == 1;
    }

    [[nodiscard]] ::timeval make_timeval(
        core_time::Duration timeout) noexcept
    {
      const auto millis = timeout.millis();

      ::timeval tv{};

      if (millis <= 0)
      {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        return tv;
      }

      tv.tv_sec =
          static_cast<decltype(tv.tv_sec)>(millis / 1000);

      tv.tv_usec =
          static_cast<decltype(tv.tv_usec)>((millis % 1000) * 1000);

      return tv;
    }

  } // namespace

  UdpSocket::UdpSocket() noexcept = default;

  UdpSocket::UdpSocket(int fd) noexcept
      : fd_(fd)
  {
  }

  UdpSocket::~UdpSocket()
  {
    close();
  }

  UdpSocket::UdpSocket(UdpSocket &&other) noexcept
      : fd_(other.fd_)
  {
    other.fd_ = -1;
  }

  UdpSocket &UdpSocket::operator=(UdpSocket &&other) noexcept
  {
    if (this != &other)
    {
      close();
      fd_ = other.fd_;
      other.fd_ = -1;
    }

    return *this;
  }

  bool UdpSocket::open()
  {
    close();

    fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);

    return fd_ >= 0;
  }

  bool UdpSocket::bind(
      const std::string &host,
      std::uint16_t port)
  {
    if (!is_valid() && !open())
    {
      return false;
    }

    ::sockaddr_in addr{};

    if (!make_sockaddr(host, port, addr))
    {
      return false;
    }

    return ::bind(
               fd_,
               reinterpret_cast<const ::sockaddr *>(&addr),
               sizeof(addr)) == 0;
  }

  std::size_t UdpSocket::send_to(
      const std::string &host,
      std::uint16_t port,
      const void *data,
      std::size_t size)
  {
    if (!is_valid() || data == nullptr || size == 0)
    {
      return 0;
    }

    ::sockaddr_in addr{};

    if (!make_sockaddr(host, port, addr))
    {
      return 0;
    }

    for (;;)
    {
      const ssize_t sent =
          ::sendto(
              fd_,
              data,
              size,
              0,
              reinterpret_cast<const ::sockaddr *>(&addr),
              sizeof(addr));

      if (sent < 0)
      {
        if (errno == EINTR)
        {
          continue;
        }

        return 0;
      }

      return static_cast<std::size_t>(sent);
    }
  }

  std::size_t UdpSocket::send_datagram(
      const utils::Datagram &datagram)
  {
    if (!datagram.is_valid())
    {
      return 0;
    }

    return send_to(
        datagram.host,
        datagram.port,
        datagram.payload.data(),
        datagram.payload.size());
  }

  utils::Datagram UdpSocket::recv_datagram(
      std::size_t max_size)
  {
    if (!is_valid() || max_size == 0)
    {
      return utils::Datagram{};
    }

    std::vector<std::uint8_t> buffer(max_size);

    ::sockaddr_in from_addr{};
    ::socklen_t from_len = sizeof(from_addr);

    for (;;)
    {
      const ssize_t received =
          ::recvfrom(
              fd_,
              buffer.data(),
              buffer.size(),
              0,
              reinterpret_cast<::sockaddr *>(&from_addr),
              &from_len);

      if (received < 0)
      {
        if (errno == EINTR)
        {
          continue;
        }

        return utils::Datagram{};
      }

      if (received == 0)
      {
        return utils::Datagram{};
      }

      char ip[INET_ADDRSTRLEN] = {0};

      if (::inet_ntop(
              AF_INET,
              &from_addr.sin_addr,
              ip,
              sizeof(ip)) == nullptr)
      {
        return utils::Datagram{};
      }

      std::vector<std::uint8_t> payload;
      payload.reserve(static_cast<std::size_t>(received));

      payload.insert(
          payload.end(),
          buffer.begin(),
          buffer.begin() + static_cast<std::ptrdiff_t>(received));

      return utils::Datagram{
          ip,
          ntohs(from_addr.sin_port),
          std::move(payload)};
    }
  }

  void UdpSocket::close() noexcept
  {
    if (fd_ < 0)
    {
      return;
    }

    ::close(fd_);

    fd_ = -1;
  }

  bool UdpSocket::set_reuse_addr(bool enabled)
  {
    if (!is_valid())
    {
      return false;
    }

    const int value = enabled ? 1 : 0;

    return ::setsockopt(
               fd_,
               SOL_SOCKET,
               SO_REUSEADDR,
               &value,
               sizeof(value)) == 0;
  }

  bool UdpSocket::set_broadcast(bool enabled)
  {
    if (!is_valid())
    {
      return false;
    }

    const int value = enabled ? 1 : 0;

    return ::setsockopt(
               fd_,
               SOL_SOCKET,
               SO_BROADCAST,
               &value,
               sizeof(value)) == 0;
  }

  bool UdpSocket::set_recv_timeout(
      core_time::Duration timeout)
  {
    if (!is_valid())
    {
      return false;
    }

    const auto tv = make_timeval(timeout);

    return ::setsockopt(
               fd_,
               SOL_SOCKET,
               SO_RCVTIMEO,
               &tv,
               sizeof(tv)) == 0;
  }

  bool UdpSocket::set_send_timeout(
      core_time::Duration timeout)
  {
    if (!is_valid())
    {
      return false;
    }

    const auto tv = make_timeval(timeout);

    return ::setsockopt(
               fd_,
               SOL_SOCKET,
               SO_SNDTIMEO,
               &tv,
               sizeof(tv)) == 0;
  }

  bool UdpSocket::is_valid() const noexcept
  {
    return fd_ >= 0;
  }

  int UdpSocket::fd() const noexcept
  {
    return fd_;
  }

} // namespace softadastra::discovery::platform::os_linux
