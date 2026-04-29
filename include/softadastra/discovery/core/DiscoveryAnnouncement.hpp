/**
 *
 *  @file DiscoveryAnnouncement.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_ANNOUNCEMENT_HPP
#define SOFTADASTRA_DISCOVERY_ANNOUNCEMENT_HPP

#include <cstdint>
#include <string>
#include <utility>

#include <softadastra/core/Core.hpp>

namespace softadastra::discovery::core
{
  namespace core_time = softadastra::core::time;

  /**
   * @brief Peer presence announcement payload.
   *
   * DiscoveryAnnouncement contains the minimal identity and reachability
   * information advertised by one node to other peers.
   *
   * It is used by:
   * - DiscoveryMessage
   * - DiscoveryEncoder
   * - DiscoveryDecoder
   * - DiscoveryRegistry
   * - DiscoveryEngine
   *
   * Fields:
   * - node_id identifies the announcing node.
   * - host identifies the announced network host.
   * - port identifies the announced transport port.
   * - timestamp identifies when the announcement was created.
   */
  struct DiscoveryAnnouncement
  {
    /**
     * @brief Logical node identifier.
     */
    std::string node_id{};

    /**
     * @brief Announced host or IP address.
     */
    std::string host{"127.0.0.1"};

    /**
     * @brief Announced listening port.
     */
    std::uint16_t port{0};

    /**
     * @brief Announcement timestamp.
     */
    core_time::Timestamp timestamp{};

    /**
     * @brief Creates an empty invalid announcement.
     */
    DiscoveryAnnouncement() = default;

    /**
     * @brief Creates an announcement.
     *
     * @param announced_node_id Logical node id.
     * @param announced_host Announced host.
     * @param announced_port Announced port.
     */
    DiscoveryAnnouncement(
        std::string announced_node_id,
        std::string announced_host,
        std::uint16_t announced_port)
        : node_id(std::move(announced_node_id)),
          host(std::move(announced_host)),
          port(announced_port),
          timestamp(core_time::Timestamp::now())
    {
    }

    /**
     * @brief Creates a local announcement.
     *
     * @param node_id Local node id.
     * @param port Announced transport port.
     * @return DiscoveryAnnouncement.
     */
    [[nodiscard]] static DiscoveryAnnouncement local(
        std::string node_id,
        std::uint16_t port)
    {
      return DiscoveryAnnouncement{
          std::move(node_id),
          "127.0.0.1",
          port};
    }

    /**
     * @brief Returns true if this announcement is usable.
     *
     * @return true when node id, host, port, and timestamp are valid.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return !node_id.empty() &&
             !host.empty() &&
             port != 0 &&
             timestamp.is_valid();
    }

    /**
     * @brief Returns true if the announcement points to localhost.
     *
     * @return true for 127.0.0.1 or localhost.
     */
    [[nodiscard]] bool is_localhost() const noexcept
    {
      return host == "127.0.0.1" ||
             host == "localhost";
    }

    /**
     * @brief Refreshes the announcement timestamp.
     */
    void touch() noexcept
    {
      timestamp = core_time::Timestamp::now();
    }

    /**
     * @brief Clears the announcement.
     */
    void clear() noexcept
    {
      node_id.clear();
      host.clear();
      port = 0;
      timestamp = core_time::Timestamp{};
    }
  };

} // namespace softadastra::discovery::core

#endif // SOFTADASTRA_DISCOVERY_ANNOUNCEMENT_HPP
