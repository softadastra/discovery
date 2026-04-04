/*
 * DiscoveryEnvelope.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_ENVELOPE_HPP
#define SOFTADASTRA_DISCOVERY_ENVELOPE_HPP

#include <cstdint>
#include <string>

#include <softadastra/discovery/core/DiscoveryMessage.hpp>

namespace softadastra::discovery::core
{
  /**
   * @brief Runtime wrapper around a discovery message
   *
   * Adds network metadata and runtime bookkeeping needed
   * by the discovery engine and backend.
   */
  struct DiscoveryEnvelope
  {
    /**
     * Discovery message being transported
     */
    DiscoveryMessage message;

    /**
     * Source host from which the datagram was received
     */
    std::string from_host;

    /**
     * Source port from which the datagram was received
     */
    std::uint16_t from_port{0};

    /**
     * Destination host to which the datagram should be sent
     */
    std::string to_host;

    /**
     * Destination port to which the datagram should be sent
     */
    std::uint16_t to_port{0};

    /**
     * Envelope creation timestamp in milliseconds
     */
    std::uint64_t timestamp{0};

    /**
     * Number of send attempts
     */
    std::uint32_t retry_count{0};

    /**
     * Last send attempt timestamp in milliseconds
     */
    std::uint64_t last_attempt_at{0};

    /**
     * @brief Check whether the envelope is usable
     */
    bool valid() const noexcept
    {
      return message.valid();
    }
  };

} // namespace softadastra::discovery::core

#endif
