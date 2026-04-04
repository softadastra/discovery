/*
 * DiscoveryMessage.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_MESSAGE_HPP
#define SOFTADASTRA_DISCOVERY_MESSAGE_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <softadastra/discovery/types/DiscoveryMessageType.hpp>

namespace softadastra::discovery::core
{
  namespace types = softadastra::discovery::types;

  /**
   * @brief Logical discovery message exchanged between nodes
   *
   * This is the protocol-level message before datagram wrapping.
   */
  struct DiscoveryMessage
  {
    /**
     * Semantic discovery message type
     */
    types::DiscoveryMessageType type{types::DiscoveryMessageType::Unknown};

    /**
     * Sender node identifier
     */
    std::string from_node_id;

    /**
     * Optional recipient node identifier
     */
    std::string to_node_id;

    /**
     * Correlation id used for probes and replies
     */
    std::string correlation_id;

    /**
     * Opaque payload bytes
     */
    std::vector<std::uint8_t> payload;

    /**
     * @brief Check whether the message is minimally valid
     */
    bool valid() const noexcept
    {
      return type != types::DiscoveryMessageType::Unknown &&
             !from_node_id.empty();
    }

    /**
     * @brief Return payload size in bytes
     */
    std::size_t payload_size() const noexcept
    {
      return payload.size();
    }
  };

} // namespace softadastra::discovery::core

#endif
