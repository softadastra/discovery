/**
 *
 *  @file Peer.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_PEER_HPP
#define SOFTADASTRA_DISCOVERY_PEER_HPP

#include <cstdint>
#include <string>
#include <utility>

#include <softadastra/core/Core.hpp>
#include <softadastra/discovery/peer/DiscoveredPeer.hpp>
#include <softadastra/transport/core/PeerInfo.hpp>

namespace softadastra::discovery
{
  namespace core_time = softadastra::core::time;
  namespace discovery_peer = softadastra::discovery::peer;
  namespace transport_core = softadastra::transport::core;

  /**
   * @brief Public discovered peer representation.
   *
   * Peer is the simple user-facing peer object exposed by the high-level
   * discovery API.
   *
   * It contains:
   * - node id
   * - host
   * - port
   * - last seen timestamp
   *
   * For advanced discovery state, use discovery::peer::DiscoveredPeer.
   */
  struct Peer
  {
    /**
     * @brief Logical node identifier.
     */
    std::string node_id{};

    /**
     * @brief Peer host or IP address.
     */
    std::string host{};

    /**
     * @brief Peer transport port.
     */
    std::uint16_t port{0};

    /**
     * @brief Last observed timestamp.
     */
    core_time::Timestamp last_seen_at{};

    /**
     * @brief Creates an empty invalid peer.
     */
    Peer() = default;

    /**
     * @brief Creates a public peer.
     *
     * @param peer_node_id Peer node id.
     * @param peer_host Peer host.
     * @param peer_port Peer transport port.
     */
    Peer(
        std::string peer_node_id,
        std::string peer_host,
        std::uint16_t peer_port)
        : node_id(std::move(peer_node_id)),
          host(std::move(peer_host)),
          port(peer_port),
          last_seen_at(core_time::Timestamp::now())
    {
    }

    /**
     * @brief Creates a public peer from transport peer info.
     *
     * @param peer Transport peer info.
     * @return Public peer.
     */
    [[nodiscard]] static Peer from_transport_peer(
        const transport_core::PeerInfo &peer)
    {
      Peer result{
          peer.node_id,
          peer.host,
          peer.port};

      return result;
    }

    /**
     * @brief Creates a public peer from a discovered peer.
     *
     * @param peer Discovered peer.
     * @return Public peer.
     */
    [[nodiscard]] static Peer from_discovered_peer(
        const discovery_peer::DiscoveredPeer &peer)
    {
      Peer result{
          peer.announcement.node_id,
          peer.announcement.host,
          peer.announcement.port};

      result.last_seen_at = peer.last_seen_at;

      return result;
    }

    /**
     * @brief Converts this peer to transport peer info.
     *
     * @return Transport peer info.
     */
    [[nodiscard]] transport_core::PeerInfo to_transport_peer() const
    {
      return transport_core::PeerInfo{
          node_id,
          host,
          port};
    }

    /**
     * @brief Returns true if the peer points to localhost.
     *
     * @return true for 127.0.0.1 or localhost.
     */
    [[nodiscard]] bool is_localhost() const noexcept
    {
      return host == "127.0.0.1" ||
             host == "localhost";
    }

    /**
     * @brief Returns true if the peer is usable.
     *
     * @return true when node id, host, port, and timestamp are valid.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return !node_id.empty() &&
             !host.empty() &&
             port != 0 &&
             last_seen_at.is_valid();
    }

    /**
     * @brief Backward-compatible valid alias.
     *
     * @return true when peer is valid.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return is_valid();
    }

    /**
     * @brief Updates the last seen timestamp.
     */
    void touch() noexcept
    {
      last_seen_at = core_time::Timestamp::now();
    }

    /**
     * @brief Clears the peer.
     */
    void clear() noexcept
    {
      node_id.clear();
      host.clear();
      port = 0;
      last_seen_at = core_time::Timestamp{};
    }
  };

} // namespace softadastra::discovery

#endif // SOFTADASTRA_DISCOVERY_PEER_HPP
