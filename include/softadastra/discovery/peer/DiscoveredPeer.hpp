/*
 * DiscoveredPeer.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_DISCOVERED_PEER_HPP
#define SOFTADASTRA_DISCOVERY_DISCOVERED_PEER_HPP

#include <cstdint>
#include <string>

#include <softadastra/discovery/core/DiscoveryAnnouncement.hpp>
#include <softadastra/discovery/types/DiscoveryPeerState.hpp>
#include <softadastra/transport/core/PeerInfo.hpp>

namespace softadastra::discovery::peer
{
  namespace core = softadastra::discovery::core;
  namespace types = softadastra::discovery::types;
  namespace transport_core = softadastra::transport::core;

  /**
   * @brief Runtime state for one discovered peer
   */
  struct DiscoveredPeer
  {
    /**
     * Last known peer announcement
     */
    core::DiscoveryAnnouncement announcement;

    /**
     * Current discovery state
     */
    types::DiscoveryPeerState state{types::DiscoveryPeerState::Unknown};

    /**
     * Last observed activity timestamp in milliseconds
     */
    std::uint64_t last_seen_at{0};

    /**
     * Number of discovery-related errors observed
     */
    std::uint32_t error_count{0};

    /**
     * @brief Check whether the discovered peer is usable
     */
    bool valid() const noexcept
    {
      return announcement.valid();
    }

    /**
     * @brief Return true if the peer is considered alive
     */
    bool alive() const noexcept
    {
      return state == types::DiscoveryPeerState::Alive;
    }

    /**
     * @brief Return true if the peer is stale
     */
    bool stale() const noexcept
    {
      return state == types::DiscoveryPeerState::Stale;
    }

    /**
     * @brief Return true if the peer is expired
     */
    bool expired() const noexcept
    {
      return state == types::DiscoveryPeerState::Expired;
    }

    /**
     * @brief Convert to transport peer info
     */
    transport_core::PeerInfo to_peer_info() const
    {
      transport_core::PeerInfo peer;
      peer.node_id = announcement.node_id;
      peer.host = announcement.host;
      peer.port = announcement.port;
      return peer;
    }
  };

} // namespace softadastra::discovery::peer

#endif
