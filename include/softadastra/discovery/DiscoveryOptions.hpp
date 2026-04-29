/**
 *
 *  @file DiscoveryOptions.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_OPTIONS_HPP
#define SOFTADASTRA_DISCOVERY_OPTIONS_HPP

#include <cstdint>
#include <string>
#include <utility>

#include <softadastra/core/Core.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>

namespace softadastra::discovery
{
  namespace core_time = softadastra::core::time;
  namespace discovery_core = softadastra::discovery::core;

  /**
   * @brief User-facing discovery options.
   *
   * DiscoveryOptions is the simple public configuration object used by the
   * legacy/high-level discovery API.
   *
   * It maps cleanly to discovery::core::DiscoveryConfig while keeping a small
   * and approachable surface.
   */
  struct DiscoveryOptions
  {
    /**
     * @brief Logical local node identifier.
     */
    std::string node_id{};

    /**
     * @brief Local UDP bind host.
     */
    std::string bind_host{"0.0.0.0"};

    /**
     * @brief Local UDP bind port.
     */
    std::uint16_t bind_port{9400};

    /**
     * @brief Broadcast host used for announcements and probes.
     */
    std::string broadcast_host{"255.255.255.255"};

    /**
     * @brief Broadcast port used for announcements and probes.
     */
    std::uint16_t broadcast_port{9400};

    /**
     * @brief Host announced to other peers.
     */
    std::string announce_host{"127.0.0.1"};

    /**
     * @brief Transport port announced to other peers.
     */
    std::uint16_t announce_port{0};

    /**
     * @brief Periodic announcement interval.
     */
    core_time::Duration announce_interval{
        core_time::Duration::from_seconds(3)};

    /**
     * @brief Peer expiration timeout.
     */
    core_time::Duration peer_ttl{
        core_time::Duration::from_seconds(15)};

    /**
     * @brief Enable broadcast announcements.
     */
    bool enable_broadcast{true};

    /**
     * @brief Creates default discovery options.
     */
    DiscoveryOptions() = default;

    /**
     * @brief Creates discovery options from node id and announce port.
     *
     * @param local_node_id Local node id.
     * @param transport_port Transport port announced to peers.
     */
    DiscoveryOptions(
        std::string local_node_id,
        std::uint16_t transport_port)
        : node_id(std::move(local_node_id)),
          announce_port(transport_port)
    {
    }

    /**
     * @brief Creates local development discovery options.
     *
     * @param node_id Local node id.
     * @param bind_port UDP bind port.
     * @param announce_port Transport port announced to peers.
     * @return DiscoveryOptions.
     */
    [[nodiscard]] static DiscoveryOptions local(
        std::string node_id,
        std::uint16_t bind_port,
        std::uint16_t announce_port)
    {
      DiscoveryOptions options{
          std::move(node_id),
          announce_port};

      options.bind_host = "127.0.0.1";
      options.bind_port = bind_port;
      options.broadcast_host = "127.0.0.1";
      options.broadcast_port = bind_port;
      options.announce_host = "127.0.0.1";

      return options;
    }

    /**
     * @brief Creates LAN broadcast discovery options.
     *
     * @param node_id Local node id.
     * @param bind_port UDP bind port.
     * @param announce_port Transport port announced to peers.
     * @return DiscoveryOptions.
     */
    [[nodiscard]] static DiscoveryOptions lan(
        std::string node_id,
        std::uint16_t bind_port,
        std::uint16_t announce_port)
    {
      DiscoveryOptions options{
          std::move(node_id),
          announce_port};

      options.bind_host = "0.0.0.0";
      options.bind_port = bind_port;
      options.broadcast_host = "255.255.255.255";
      options.broadcast_port = bind_port;
      options.announce_host = "127.0.0.1";
      options.enable_broadcast = true;

      return options;
    }

    /**
     * @brief Returns the announce interval in milliseconds.
     *
     * @return Milliseconds.
     */
    [[nodiscard]] core_time::Duration::rep
    announce_interval_ms() const noexcept
    {
      return announce_interval.millis();
    }

    /**
     * @brief Returns the peer TTL in milliseconds.
     *
     * @return Milliseconds.
     */
    [[nodiscard]] core_time::Duration::rep
    peer_ttl_ms() const noexcept
    {
      return peer_ttl.millis();
    }

    /**
     * @brief Converts public options to core DiscoveryConfig.
     *
     * @return DiscoveryConfig.
     */
    [[nodiscard]] discovery_core::DiscoveryConfig to_config() const
    {
      discovery_core::DiscoveryConfig config;

      config.bind_host = bind_host;
      config.bind_port = bind_port;
      config.broadcast_host = broadcast_host;
      config.broadcast_port = broadcast_port;
      config.node_id = node_id;
      config.announce_host = announce_host;
      config.announce_port = announce_port;
      config.announce_interval = announce_interval;
      config.peer_ttl = peer_ttl;
      config.enable_broadcast = enable_broadcast;

      return config;
    }

    /**
     * @brief Returns true if the options are usable.
     *
     * @return true when all required values are valid.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return !node_id.empty() &&
             !bind_host.empty() &&
             bind_port != 0 &&
             !broadcast_host.empty() &&
             broadcast_port != 0 &&
             !announce_host.empty() &&
             announce_port != 0 &&
             announce_interval.millis() > 0 &&
             peer_ttl.millis() > 0;
    }

    /**
     * @brief Backward-compatible valid alias.
     *
     * @return true when options are valid.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return is_valid();
    }
  };

} // namespace softadastra::discovery

#endif // SOFTADASTRA_DISCOVERY_OPTIONS_HPP
