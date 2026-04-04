/*
 * Datagram.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_DATAGRAM_HPP
#define SOFTADASTRA_DISCOVERY_DATAGRAM_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace softadastra::discovery::utils
{
  /**
   * @brief Simple UDP datagram container
   *
   * Holds raw payload bytes and optional addressing metadata
   * used by the discovery backend.
   */
  struct Datagram
  {
    /**
     * Remote host or IP address
     */
    std::string host;

    /**
     * Remote port
     */
    std::uint16_t port{0};

    /**
     * Raw datagram payload
     */
    std::vector<std::uint8_t> payload;

    /**
     * @brief Return payload size in bytes
     */
    std::size_t payload_size() const noexcept
    {
      return payload.size();
    }

    /**
     * @brief Check whether the datagram is internally usable
     */
    bool valid() const noexcept
    {
      return !host.empty() &&
             port != 0 &&
             !payload.empty();
    }
  };

} // namespace softadastra::discovery::utils

#endif
