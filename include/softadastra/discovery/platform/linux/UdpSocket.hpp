/**
 *
 *  @file UdpSocket.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_UDP_SOCKET_HPP
#define SOFTADASTRA_DISCOVERY_UDP_SOCKET_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include <softadastra/core/Core.hpp>
#include <softadastra/discovery/utils/Datagram.hpp>

namespace softadastra::discovery::platform::os_linux
{
  namespace utils = softadastra::discovery::utils;
  namespace core_time = softadastra::core::time;

  /**
   * @brief Minimal Linux UDP socket wrapper.
   *
   * UdpSocket is a small RAII wrapper around an IPv4 UDP socket.
   *
   * It is used by the discovery module to implement UDP-based local peer
   * discovery.
   *
   * Current scope:
   * - open / close
   * - bind
   * - send datagram
   * - receive datagram
   * - broadcast support
   * - basic socket options
   *
   * The socket owns its file descriptor and closes it in the destructor.
   */
  class UdpSocket
  {
  public:
    /**
     * @brief Creates an invalid socket.
     */
    UdpSocket() noexcept;

    /**
     * @brief Adopts an existing socket file descriptor.
     *
     * @param fd Existing socket file descriptor.
     */
    explicit UdpSocket(int fd) noexcept;

    /**
     * @brief Closes the socket if it is still open.
     */
    ~UdpSocket();

    UdpSocket(const UdpSocket &) = delete;
    UdpSocket &operator=(const UdpSocket &) = delete;

    /**
     * @brief Move-constructs a socket.
     *
     * Ownership of the file descriptor is transferred.
     *
     * @param other Source socket.
     */
    UdpSocket(UdpSocket &&other) noexcept;

    /**
     * @brief Move-assigns a socket.
     *
     * Any currently owned descriptor is closed before taking ownership.
     *
     * @param other Source socket.
     * @return This socket.
     */
    UdpSocket &operator=(UdpSocket &&other) noexcept;

    /**
     * @brief Opens a new IPv4 UDP socket.
     *
     * @return true on success.
     */
    [[nodiscard]] bool open();

    /**
     * @brief Binds the socket to host and port.
     *
     * @param host Local bind host.
     * @param port Local bind port.
     * @return true on success.
     */
    [[nodiscard]] bool bind(
        const std::string &host,
        std::uint16_t port);

    /**
     * @brief Sends bytes to host and port.
     *
     * @param host Destination host.
     * @param port Destination port.
     * @param data Source buffer.
     * @param size Number of bytes to send.
     * @return Number of bytes successfully sent.
     */
    [[nodiscard]] std::size_t send_to(
        const std::string &host,
        std::uint16_t port,
        const void *data,
        std::size_t size);

    /**
     * @brief Sends one discovery datagram.
     *
     * @param datagram Datagram to send.
     * @return Number of bytes successfully sent.
     */
    [[nodiscard]] std::size_t send_datagram(
        const utils::Datagram &datagram);

    /**
     * @brief Receives one datagram.
     *
     * Returns an invalid Datagram on failure.
     *
     * @param max_size Maximum number of payload bytes to receive.
     * @return Received datagram.
     */
    [[nodiscard]] utils::Datagram recv_datagram(
        std::size_t max_size);

    /**
     * @brief Closes the socket.
     */
    void close() noexcept;

    /**
     * @brief Enables or disables SO_REUSEADDR.
     *
     * @param enabled Whether the option should be enabled.
     * @return true on success.
     */
    [[nodiscard]] bool set_reuse_addr(bool enabled);

    /**
     * @brief Enables or disables SO_BROADCAST.
     *
     * @param enabled Whether the option should be enabled.
     * @return true on success.
     */
    [[nodiscard]] bool set_broadcast(bool enabled);

    /**
     * @brief Sets receive timeout.
     *
     * @param timeout Timeout duration.
     * @return true on success.
     */
    [[nodiscard]] bool set_recv_timeout(
        core_time::Duration timeout);

    /**
     * @brief Sets send timeout.
     *
     * @param timeout Timeout duration.
     * @return true on success.
     */
    [[nodiscard]] bool set_send_timeout(
        core_time::Duration timeout);

    /**
     * @brief Sets receive timeout in milliseconds.
     *
     * @param timeout_ms Timeout in milliseconds.
     * @return true on success.
     */
    [[nodiscard]] bool set_recv_timeout_ms(std::uint64_t timeout_ms)
    {
      return set_recv_timeout(
          core_time::Duration::from_millis(
              static_cast<core_time::Duration::rep>(timeout_ms)));
    }

    /**
     * @brief Sets send timeout in milliseconds.
     *
     * @param timeout_ms Timeout in milliseconds.
     * @return true on success.
     */
    [[nodiscard]] bool set_send_timeout_ms(std::uint64_t timeout_ms)
    {
      return set_send_timeout(
          core_time::Duration::from_millis(
              static_cast<core_time::Duration::rep>(timeout_ms)));
    }

    /**
     * @brief Returns whether the socket owns a valid file descriptor.
     *
     * @return true when valid.
     */
    [[nodiscard]] bool is_valid() const noexcept;

    /**
     * @brief Backward-compatible valid alias.
     *
     * @return true when valid.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return is_valid();
    }

    /**
     * @brief Returns the raw file descriptor.
     *
     * @return File descriptor, or -1 if invalid.
     */
    [[nodiscard]] int fd() const noexcept;

  private:
    int fd_{-1};
  };

} // namespace softadastra::discovery::platform::os_linux

#endif // SOFTADASTRA_DISCOVERY_UDP_SOCKET_HPP
