/*
 * UdpSocket.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_UDP_SOCKET_HPP
#define SOFTADASTRA_DISCOVERY_UDP_SOCKET_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <softadastra/discovery/utils/Datagram.hpp>

namespace softadastra::discovery::platform::os_linux
{
  namespace utils = softadastra::discovery::utils;

  /**
   * @brief Minimal Linux UDP socket wrapper
   *
   * This class provides a small RAII wrapper around an IPv4 UDP socket
   * for the discovery module.
   *
   * Current scope:
   * - open / close
   * - bind
   * - send datagram
   * - receive datagram
   * - broadcast support
   * - basic socket options
   */
  class UdpSocket
  {
  public:
    /**
     * @brief Create an invalid socket
     */
    UdpSocket() noexcept;

    /**
     * @brief Adopt an existing socket file descriptor
     */
    explicit UdpSocket(int fd) noexcept;

    /**
     * @brief Destructor closes the socket if still open
     */
    ~UdpSocket();

    UdpSocket(const UdpSocket &) = delete;
    UdpSocket &operator=(const UdpSocket &) = delete;

    UdpSocket(UdpSocket &&other) noexcept;
    UdpSocket &operator=(UdpSocket &&other) noexcept;

    /**
     * @brief Open a new IPv4 UDP socket
     */
    bool open();

    /**
     * @brief Bind the socket to host:port
     */
    bool bind(const std::string &host, std::uint16_t port);

    /**
     * @brief Send one datagram to host:port
     *
     * Returns the number of bytes successfully sent.
     */
    std::size_t send_to(const std::string &host,
                        std::uint16_t port,
                        const void *data,
                        std::size_t size);

    /**
     * @brief Send one discovery datagram
     *
     * Returns the number of bytes successfully sent.
     */
    std::size_t send_datagram(const utils::Datagram &datagram);

    /**
     * @brief Receive one datagram
     *
     * Returns an invalid datagram on failure.
     */
    utils::Datagram recv_datagram(std::size_t max_size);

    /**
     * @brief Close the socket
     */
    void close() noexcept;

    /**
     * @brief Enable or disable SO_REUSEADDR
     */
    bool set_reuse_addr(bool enabled);

    /**
     * @brief Enable or disable SO_BROADCAST
     */
    bool set_broadcast(bool enabled);

    /**
     * @brief Set receive timeout in milliseconds
     */
    bool set_recv_timeout_ms(std::uint64_t timeout_ms);

    /**
     * @brief Set send timeout in milliseconds
     */
    bool set_send_timeout_ms(std::uint64_t timeout_ms);

    /**
     * @brief Return whether the socket is valid
     */
    bool valid() const noexcept;

    /**
     * @brief Return the raw file descriptor
     */
    int fd() const noexcept;

  private:
    int fd_;
  };

} // namespace softadastra::discovery::platform::linux

#endif
