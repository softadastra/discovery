/*
 * DiscoveryPeerState.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_PEER_STATE_HPP
#define SOFTADASTRA_DISCOVERY_PEER_STATE_HPP

#include <cstdint>

namespace softadastra::discovery::types
{
  /**
   * @brief Discovery state of a known peer
   */
  enum class DiscoveryPeerState : std::uint8_t
  {
    Unknown = 0,
    Discovered,
    Alive,
    Stale,
    Expired
  };

} // namespace softadastra::discovery::types

#endif
