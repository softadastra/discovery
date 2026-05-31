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

#include <softadastra/discovery/platform/windows/UdpSocket.hpp>

#include <algorithm>
#include <climits>
#include <cstdint>
#include <vector>
#include <ws2tcpip.h>

namespace softadastra::discovery::platform::os_windows
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

    [[nodiscard]] DWORD make_timeout_ms(
        core_time::Duration timeout) noexcept
    {
      const auto millis = timeout.millis();

      if (millis <= 0)
      {
        return 0;
      }

      const auto max_value =
          static_cast<core_time::Duration::rep>(UINT32_MAX);

      if (millis > max_value)
      {
        return UINT32_MAX;
      }

      return static_cast<DWORD>(millis);
    }

    [[nodiscard]] int chunk_size(std::size_t remaining) noexcept
    {
      const auto max_int =
          static_cast<std::size_t>(INT_MAX);

      return static_cast<int>(std::min(remaining, max_int));
    }
  } // namespace

  UdpSocket::UdpSocket() noexcept = default;

  UdpSocket::UdpSocket(SOCKET socket) noexcept
      : socket_(socket)
  {
  }

  UdpSocket::~UdpSocket()
  {
    close();
  }

  UdpSocket::UdpSocket(UdpSocket &&other) noexcept
      : socket_(other.socket_)
  {
    other.socket_ = INVALID_SOCKET;
  }

  UdpSocket &UdpSocket::operator=(UdpSocket &&other) noexcept
  {
    if (this != &other)
    {
      close();
      socket_ = other.socket_;
      other.socket_ = INVALID_SOCKET;
    }

    return *this;
  }

  bool UdpSocket::open()
  {
    close();

    socket_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    return socket_ != INVALID_SOCKET;
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
               socket_,
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

    const int sent =
        ::sendto(
            socket_,
            static_cast<const char *>(data),
            chunk_size(size),
            0,
            reinterpret_cast<const ::sockaddr *>(&addr),
            sizeof(addr));

    if (sent == SOCKET_ERROR)
    {
      return 0;
    }

    return static_cast<std::size_t>(sent);
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
    int from_len = sizeof(from_addr);

    const int received =
        ::recvfrom(
            socket_,
            reinterpret_cast<char *>(buffer.data()),
            chunk_size(buffer.size()),
            0,
            reinterpret_cast<::sockaddr *>(&from_addr),
            &from_len);

    if (received == SOCKET_ERROR || received == 0)
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

  void UdpSocket::close() noexcept
  {
    if (socket_ == INVALID_SOCKET)
    {
      return;
    }

    ::closesocket(socket_);

    socket_ = INVALID_SOCKET;
  }

  bool UdpSocket::set_reuse_addr(bool enabled)
  {
    if (!is_valid())
    {
      return false;
    }

    const BOOL value = enabled ? TRUE : FALSE;

    return ::setsockopt(
               socket_,
               SOL_SOCKET,
               SO_REUSEADDR,
               reinterpret_cast<const char *>(&value),
               sizeof(value)) == 0;
  }

  bool UdpSocket::set_broadcast(bool enabled)
  {
    if (!is_valid())
    {
      return false;
    }

    const BOOL value = enabled ? TRUE : FALSE;

    return ::setsockopt(
               socket_,
               SOL_SOCKET,
               SO_BROADCAST,
               reinterpret_cast<const char *>(&value),
               sizeof(value)) == 0;
  }

  bool UdpSocket::set_recv_timeout(
      core_time::Duration timeout)
  {
    if (!is_valid())
    {
      return false;
    }

    const DWORD value = make_timeout_ms(timeout);

    return ::setsockopt(
               socket_,
               SOL_SOCKET,
               SO_RCVTIMEO,
               reinterpret_cast<const char *>(&value),
               sizeof(value)) == 0;
  }

  bool UdpSocket::set_send_timeout(
      core_time::Duration timeout)
  {
    if (!is_valid())
    {
      return false;
    }

    const DWORD value = make_timeout_ms(timeout);

    return ::setsockopt(
               socket_,
               SOL_SOCKET,
               SO_SNDTIMEO,
               reinterpret_cast<const char *>(&value),
               sizeof(value)) == 0;
  }

  bool UdpSocket::is_valid() const noexcept
  {
    return socket_ != INVALID_SOCKET;
  }

  SOCKET UdpSocket::native_handle() const noexcept
  {
    return socket_;
  }

} // namespace softadastra::discovery::platform::os_windows
