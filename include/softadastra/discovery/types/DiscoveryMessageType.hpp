/*
 * DiscoveryMessageType.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_MESSAGE_TYPE_HPP
#define SOFTADASTRA_DISCOVERY_MESSAGE_TYPE_HPP

#include <cstdint>

namespace softadastra::discovery::types
{
  /**
   * @brief Discovery-level message type
   *
   * Defines the semantic purpose of a discovery message
   * exchanged between nodes.
   */
  enum class DiscoveryMessageType : std::uint8_t
  {
    Unknown = 0,

    Announce, // periodic peer presence announcement
    Probe,    // discovery probe request
    Reply     // direct reply to a probe
  };

} // namespace softadastra::discovery::types

#endif
