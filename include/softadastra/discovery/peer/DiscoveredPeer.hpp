/**
 *
 *  @file DiscoveredPeer.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_DISCOVERED_PEER_HPP
#define SOFTADASTRA_DISCOVERY_DISCOVERED_PEER_HPP

#include <cstdint>
#include <utility>

#include <softadastra/core/Core.hpp>
#include <softadastra/discovery/core/DiscoveryAnnouncement.hpp>
#include <softadastra/discovery/types/DiscoveryPeerState.hpp>
#include <softadastra/transport/core/PeerInfo.hpp>

namespace softadastra::discovery::peer
{
  namespace core = softadastra::discovery::core;
  namespace types = softadastra::discovery::types;
  namespace transport_core = softadastra::transport::core;
  namespace core_time = softadastra::core::time;

  /**
   * @brief Runtime state for one discovered peer.
   *
   * DiscoveredPeer combines the last known announcement with runtime discovery
   * state.
   *
   * It tracks:
   * - last announcement
   * - current discovery state
   * - last seen timestamp
   * - error count
   *
   * It can also be converted to transport::core::PeerInfo so the transport
   * module can connect to the peer.
   */
  struct DiscoveredPeer
  {
    /**
     * @brief Last known peer announcement.
     */
    core::DiscoveryAnnouncement announcement{};

    /**
     * @brief Current discovery state.
     */
    types::DiscoveryPeerState state{types::DiscoveryPeerState::Unknown};

    /**
     * @brief Last observed activity timestamp.
     */
    core_time::Timestamp last_seen_at{};

    /**
     * @brief Number of discovery-related errors observed.
     */
    std::uint32_t error_count{0};

    /**
     * @brief Creates an empty invalid discovered peer.
     */
    DiscoveredPeer() = default;

    /**
     * @brief Creates a discovered peer from an announcement.
     *
     * @param peer_announcement Discovery announcement.
     */
    explicit DiscoveredPeer(core::DiscoveryAnnouncement peer_announcement)
        : announcement(std::move(peer_announcement)),
          state(types::DiscoveryPeerState::Discovered),
          last_seen_at(core_time::Timestamp::now())
    {
    }

    /**
     * @brief Returns the peer node id.
     *
     * @return Peer node id.
     */
    [[nodiscard]] const std::string &node_id() const noexcept
    {
      return announcement.node_id;
    }

    /**
     * @brief Returns true if the discovered peer is usable.
     *
     * @return true when announcement and state are valid.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return announcement.is_valid() &&
             types::is_valid(state);
    }

    /**
     * @brief Returns true if the peer is considered available.
     *
     * @return true when state is Discovered or Alive.
     */
    [[nodiscard]] bool available() const noexcept
    {
      return types::is_available(state);
    }

    /**
     * @brief Returns true if the peer has been discovered.
     *
     * @return true when state is Discovered.
     */
    [[nodiscard]] bool discovered() const noexcept
    {
      return state == types::DiscoveryPeerState::Discovered;
    }

    /**
     * @brief Returns true if the peer is considered alive.
     *
     * @return true when state is Alive.
     */
    [[nodiscard]] bool alive() const noexcept
    {
      return state == types::DiscoveryPeerState::Alive;
    }

    /**
     * @brief Returns true if the peer is stale.
     *
     * @return true when state is Stale.
     */
    [[nodiscard]] bool stale() const noexcept
    {
      return state == types::DiscoveryPeerState::Stale;
    }

    /**
     * @brief Returns true if the peer is expired.
     *
     * @return true when state is Expired.
     */
    [[nodiscard]] bool expired() const noexcept
    {
      return state == types::DiscoveryPeerState::Expired;
    }

    /**
     * @brief Marks the peer as discovered.
     */
    void mark_discovered() noexcept
    {
      state = types::DiscoveryPeerState::Discovered;
      touch();
    }

    /**
     * @brief Marks the peer as alive.
     */
    void mark_alive() noexcept
    {
      state = types::DiscoveryPeerState::Alive;
      touch();
    }

    /**
     * @brief Marks the peer as stale.
     */
    void mark_stale() noexcept
    {
      state = types::DiscoveryPeerState::Stale;
      touch();
    }

    /**
     * @brief Marks the peer as expired.
     */
    void mark_expired() noexcept
    {
      state = types::DiscoveryPeerState::Expired;
      touch();
    }

    /**
     * @brief Marks an error and increments the error count.
     */
    void mark_error() noexcept
    {
      ++error_count;
      touch();
    }

    /**
     * @brief Updates the last observed timestamp.
     */
    void touch() noexcept
    {
      last_seen_at = core_time::Timestamp::now();
    }

    /**
     * @brief Updates the peer from a fresh announcement.
     *
     * @param peer_announcement New announcement.
     */
    void update(core::DiscoveryAnnouncement peer_announcement)
    {
      announcement = std::move(peer_announcement);
      mark_alive();
    }

    /**
     * @brief Converts this discovered peer to transport peer info.
     *
     * @return Transport peer info.
     */
    [[nodiscard]] transport_core::PeerInfo to_peer_info() const
    {
      return transport_core::PeerInfo{
          announcement.node_id,
          announcement.host,
          announcement.port};
    }

    /**
     * @brief Clears the discovered peer.
     */
    void clear() noexcept
    {
      announcement.clear();
      state = types::DiscoveryPeerState::Unknown;
      last_seen_at = core_time::Timestamp{};
      error_count = 0;
    }
  };

} // namespace softadastra::discovery::peer

#endif // SOFTADASTRA_DISCOVERY_DISCOVERED_PEER_HPP
