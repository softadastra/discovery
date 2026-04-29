/**
 *
 *  @file DiscoveryPeerState.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_PEER_STATE_HPP
#define SOFTADASTRA_DISCOVERY_PEER_STATE_HPP

#include <cstdint>
#include <string_view>

namespace softadastra::discovery::types
{
  /**
   * @brief Runtime discovery state of a known peer.
   *
   * DiscoveryPeerState tracks the lifecycle of a peer inside the discovery
   * module.
   *
   * It is used by:
   * - DiscoveredPeer
   * - DiscoveryRegistry
   * - DiscoveryEngine
   * - diagnostics and metrics
   *
   * Rules:
   * - Values must remain stable over time.
   * - Do not reorder existing values.
   * - Do not remove existing values once released.
   * - Add new values only at the end.
   */
  enum class DiscoveryPeerState : std::uint8_t
  {
    /**
     * @brief Unknown or invalid peer state.
     */
    Unknown = 0,

    /**
     * @brief Peer has been discovered at least once.
     */
    Discovered,

    /**
     * @brief Peer is considered alive and recently observed.
     */
    Alive,

    /**
     * @brief Peer has not been seen recently but is not expired yet.
     */
    Stale,

    /**
     * @brief Peer has exceeded its TTL and should be ignored or removed.
     */
    Expired
  };

  /**
   * @brief Returns a stable string representation of a discovery peer state.
   *
   * @param state Discovery peer state.
   * @return Stable string representation.
   */
  [[nodiscard]] constexpr std::string_view
  to_string(DiscoveryPeerState state) noexcept
  {
    switch (state)
    {
    case DiscoveryPeerState::Unknown:
      return "unknown";

    case DiscoveryPeerState::Discovered:
      return "discovered";

    case DiscoveryPeerState::Alive:
      return "alive";

    case DiscoveryPeerState::Stale:
      return "stale";

    case DiscoveryPeerState::Expired:
      return "expired";

    default:
      return "invalid";
    }
  }

  /**
   * @brief Returns true if the peer state is known and usable.
   *
   * Unknown is intentionally treated as invalid.
   *
   * @param state Discovery peer state.
   * @return true for usable peer states.
   */
  [[nodiscard]] constexpr bool is_valid(DiscoveryPeerState state) noexcept
  {
    return state == DiscoveryPeerState::Discovered ||
           state == DiscoveryPeerState::Alive ||
           state == DiscoveryPeerState::Stale ||
           state == DiscoveryPeerState::Expired;
  }

  /**
   * @brief Returns true if the peer should be considered reachable.
   *
   * @param state Discovery peer state.
   * @return true for Discovered and Alive.
   */
  [[nodiscard]] constexpr bool is_available(DiscoveryPeerState state) noexcept
  {
    return state == DiscoveryPeerState::Discovered ||
           state == DiscoveryPeerState::Alive;
  }

  /**
   * @brief Returns true if the peer should not be used for new connections.
   *
   * @param state Discovery peer state.
   * @return true for Stale, Expired, and Unknown.
   */
  [[nodiscard]] constexpr bool is_unavailable(DiscoveryPeerState state) noexcept
  {
    return state == DiscoveryPeerState::Unknown ||
           state == DiscoveryPeerState::Stale ||
           state == DiscoveryPeerState::Expired;
  }

  /**
   * @brief Returns true if the peer has expired.
   *
   * @param state Discovery peer state.
   * @return true for Expired.
   */
  [[nodiscard]] constexpr bool is_expired(DiscoveryPeerState state) noexcept
  {
    return state == DiscoveryPeerState::Expired;
  }

} // namespace softadastra::discovery::types

#endif // SOFTADASTRA_DISCOVERY_PEER_STATE_HPP
