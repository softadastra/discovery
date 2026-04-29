/**
 *
 *  @file Datagram.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_DATAGRAM_HPP
#define SOFTADASTRA_DISCOVERY_DATAGRAM_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace softadastra::discovery::utils
{
  /**
   * @brief UDP datagram container.
   *
   * Datagram holds raw payload bytes and addressing metadata used by the
   * discovery backend.
   *
   * It contains:
   * - remote host
   * - remote port
   * - raw datagram payload
   *
   * It does not interpret payload bytes.
   * DiscoveryEncoder and DiscoveryDecoder handle protocol-level encoding.
   */
  struct Datagram
  {
    /**
     * @brief Remote host or IP address.
     */
    std::string host{};

    /**
     * @brief Remote port.
     */
    std::uint16_t port{0};

    /**
     * @brief Raw datagram payload.
     */
    std::vector<std::uint8_t> payload{};

    /**
     * @brief Creates an empty invalid datagram.
     */
    Datagram() = default;

    /**
     * @brief Creates a datagram.
     *
     * @param remote_host Remote host or IP address.
     * @param remote_port Remote port.
     * @param payload_bytes Raw payload bytes.
     */
    Datagram(
        std::string remote_host,
        std::uint16_t remote_port,
        std::vector<std::uint8_t> payload_bytes)
        : host(std::move(remote_host)),
          port(remote_port),
          payload(std::move(payload_bytes))
    {
    }

    /**
     * @brief Returns true if the datagram has destination metadata.
     *
     * @return true when host and port are present.
     */
    [[nodiscard]] bool has_endpoint() const noexcept
    {
      return !host.empty() &&
             port != 0;
    }

    /**
     * @brief Returns true if the datagram has payload bytes.
     *
     * @return true when payload is not empty.
     */
    [[nodiscard]] bool has_payload() const noexcept
    {
      return !payload.empty();
    }

    /**
     * @brief Returns payload size in bytes.
     *
     * @return Payload byte count.
     */
    [[nodiscard]] std::size_t payload_size() const noexcept
    {
      return payload.size();
    }

    /**
     * @brief Returns true if the datagram is usable.
     *
     * @return true when endpoint metadata and payload are present.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return has_endpoint() &&
             has_payload();
    }

    /**
     * @brief Clears the datagram.
     */
    void clear() noexcept
    {
      host.clear();
      port = 0;
      payload.clear();
    }
  };

} // namespace softadastra::discovery::utils

#endif // SOFTADASTRA_DISCOVERY_DATAGRAM_HPP
