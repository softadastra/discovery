/**
 *
 *  @file DiscoveryConfig.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_CONFIG_HPP
#define SOFTADASTRA_DISCOVERY_CONFIG_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include <softadastra/core/Core.hpp>

namespace softadastra::discovery::core
{
  namespace core_time = softadastra::core::time;

  /**
   * @brief Runtime configuration for the discovery layer.
   *
   * DiscoveryConfig controls how the discovery module binds locally,
   * broadcasts announcements, and expires known peers.
   *
   * It controls:
   * - local bind address
   * - local bind port
   * - broadcast address
   * - broadcast port
   * - local node id
   * - public announced host
   * - public announced port
   * - announce interval
   * - peer TTL
   * - maximum datagram size
   * - broadcast mode
   */
  struct DiscoveryConfig
  {
    /**
     * @brief Local bind host.
     */
    std::string bind_host{"0.0.0.0"};

    /**
     * @brief Local bind port.
     */
    std::uint16_t bind_port{0};

    /**
     * @brief Broadcast host used for peer announcements.
     */
    std::string broadcast_host{"255.255.255.255"};

    /**
     * @brief Broadcast port used for peer announcements.
     */
    std::uint16_t broadcast_port{0};

    /**
     * @brief Logical local node identifier.
     */
    std::string node_id{};

    /**
     * @brief Public host announced to other peers.
     */
    std::string announce_host{"127.0.0.1"};

    /**
     * @brief Public port announced to other peers.
     */
    std::uint16_t announce_port{0};

    /**
     * @brief Periodic announce interval.
     */
    core_time::Duration announce_interval{
        core_time::Duration::from_seconds(3)};

    /**
     * @brief Peer expiration timeout.
     */
    core_time::Duration peer_ttl{
        core_time::Duration::from_seconds(15)};

    /**
     * @brief Maximum allowed datagram payload size in bytes.
     */
    std::size_t max_datagram_size{64 * 1024};

    /**
     * @brief Enable broadcast announcements.
     */
    bool enable_broadcast{true};

    /**
     * @brief Creates a default invalid discovery configuration.
     */
    DiscoveryConfig() = default;

    /**
     * @brief Creates a discovery configuration from node id and ports.
     *
     * @param local_node_id Logical node id.
     * @param bind_port_value Local UDP bind port.
     * @param announce_port_value Public announced transport port.
     */
    DiscoveryConfig(
        std::string local_node_id,
        std::uint16_t bind_port_value,
        std::uint16_t announce_port_value)
        : bind_port(bind_port_value),
          broadcast_port(bind_port_value),
          node_id(std::move(local_node_id)),
          announce_port(announce_port_value)
    {
    }

    /**
     * @brief Creates a local development discovery configuration.
     *
     * @param node_id Local node id.
     * @param bind_port UDP bind port.
     * @param announce_port Transport port announced to peers.
     * @return DiscoveryConfig.
     */
    [[nodiscard]] static DiscoveryConfig local(
        std::string node_id,
        std::uint16_t bind_port,
        std::uint16_t announce_port)
    {
      DiscoveryConfig config{
          std::move(node_id),
          bind_port,
          announce_port};

      config.bind_host = "127.0.0.1";
      config.broadcast_host = "127.0.0.1";
      config.announce_host = "127.0.0.1";

      return config;
    }

    /**
     * @brief Creates a LAN broadcast discovery configuration.
     *
     * @param node_id Local node id.
     * @param bind_port UDP bind port.
     * @param announce_port Transport port announced to peers.
     * @return DiscoveryConfig.
     */
    [[nodiscard]] static DiscoveryConfig lan(
        std::string node_id,
        std::uint16_t bind_port,
        std::uint16_t announce_port)
    {
      DiscoveryConfig config{
          std::move(node_id),
          bind_port,
          announce_port};

      config.bind_host = "0.0.0.0";
      config.broadcast_host = "255.255.255.255";
      config.announce_host = "127.0.0.1";
      config.enable_broadcast = true;

      return config;
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
     * @brief Returns true if the configuration is usable.
     *
     * @return true when bind, broadcast, node, announce, timing, and size
     * values are valid.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return !bind_host.empty() &&
             bind_port != 0 &&
             !broadcast_host.empty() &&
             broadcast_port != 0 &&
             !node_id.empty() &&
             !announce_host.empty() &&
             announce_port != 0 &&
             announce_interval.millis() > 0 &&
             peer_ttl.millis() > 0 &&
             max_datagram_size > 0;
    }
  };

} // namespace softadastra::discovery::core

#endif // SOFTADASTRA_DISCOVERY_CONFIG_HPP
