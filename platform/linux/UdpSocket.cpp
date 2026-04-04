#include <softadastra/discovery/platform/linux/UdpSocket.hpp>

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace softadastra::discovery::platform::os_linux
{
  namespace
  {
    ::sockaddr_in make_sockaddr(const std::string &host, std::uint16_t port)
    {
      ::sockaddr_in addr{};
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);

      if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1)
      {
        throw std::runtime_error("UdpSocket: invalid IPv4 address: " + host);
      }

      return addr;
    }

    ::timeval make_timeval(std::uint64_t timeout_ms)
    {
      ::timeval tv{};
      tv.tv_sec = static_cast<decltype(tv.tv_sec)>(timeout_ms / 1000ULL);
      tv.tv_usec = static_cast<decltype(tv.tv_usec)>((timeout_ms % 1000ULL) * 1000ULL);
      return tv;
    }
  } // namespace

  UdpSocket::UdpSocket() noexcept
      : fd_(-1)
  {
  }

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

  bool UdpSocket::bind(const std::string &host, std::uint16_t port)
  {
    if (!valid() && !open())
    {
      return false;
    }

    const auto addr = make_sockaddr(host, port);

    return ::bind(fd_,
                  reinterpret_cast<const ::sockaddr *>(&addr),
                  sizeof(addr)) == 0;
  }

  std::size_t UdpSocket::send_to(const std::string &host,
                                 std::uint16_t port,
                                 const void *data,
                                 std::size_t size)
  {
    if (!valid() || data == nullptr || size == 0)
    {
      return 0;
    }

    const auto addr = make_sockaddr(host, port);

    for (;;)
    {
      const ssize_t sent = ::sendto(fd_,
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

  std::size_t UdpSocket::send_datagram(const utils::Datagram &datagram)
  {
    if (!datagram.valid())
    {
      return 0;
    }

    return send_to(datagram.host,
                   datagram.port,
                   datagram.payload.data(),
                   datagram.payload.size());
  }

  utils::Datagram UdpSocket::recv_datagram(std::size_t max_size)
  {
    utils::Datagram datagram;

    if (!valid() || max_size == 0)
    {
      return datagram;
    }

    std::vector<std::uint8_t> buffer(max_size);

    ::sockaddr_in from_addr{};
    ::socklen_t from_len = sizeof(from_addr);

    for (;;)
    {
      const ssize_t received = ::recvfrom(
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
      if (::inet_ntop(AF_INET, &from_addr.sin_addr, ip, sizeof(ip)) == nullptr)
      {
        return utils::Datagram{};
      }

      datagram.host = ip;
      datagram.port = ntohs(from_addr.sin_port);
      datagram.payload.assign(buffer.begin(),
                              buffer.begin() + static_cast<std::size_t>(received));

      return datagram;
    }
  }

  void UdpSocket::close() noexcept
  {
    if (fd_ >= 0)
    {
      ::close(fd_);
      fd_ = -1;
    }
  }

  bool UdpSocket::set_reuse_addr(bool enabled)
  {
    if (!valid())
    {
      return false;
    }

    const int value = enabled ? 1 : 0;

    return ::setsockopt(fd_,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        &value,
                        sizeof(value)) == 0;
  }

  bool UdpSocket::set_broadcast(bool enabled)
  {
    if (!valid())
    {
      return false;
    }

    const int value = enabled ? 1 : 0;

    return ::setsockopt(fd_,
                        SOL_SOCKET,
                        SO_BROADCAST,
                        &value,
                        sizeof(value)) == 0;
  }

  bool UdpSocket::set_recv_timeout_ms(std::uint64_t timeout_ms)
  {
    if (!valid())
    {
      return false;
    }

    const auto tv = make_timeval(timeout_ms);

    return ::setsockopt(fd_,
                        SOL_SOCKET,
                        SO_RCVTIMEO,
                        &tv,
                        sizeof(tv)) == 0;
  }

  bool UdpSocket::set_send_timeout_ms(std::uint64_t timeout_ms)
  {
    if (!valid())
    {
      return false;
    }

    const auto tv = make_timeval(timeout_ms);

    return ::setsockopt(fd_,
                        SOL_SOCKET,
                        SO_SNDTIMEO,
                        &tv,
                        sizeof(tv)) == 0;
  }

  bool UdpSocket::valid() const noexcept
  {
    return fd_ >= 0;
  }

  int UdpSocket::fd() const noexcept
  {
    return fd_;
  }

} // namespace softadastra::discovery::platform::linux
