/**
 *
 *  @file DiscoveryMessageType.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_MESSAGE_TYPE_HPP
#define SOFTADASTRA_DISCOVERY_MESSAGE_TYPE_HPP

#include <cstdint>
#include <string_view>

namespace softadastra::discovery::types
{
  /**
   * @brief Semantic type of a discovery message.
   *
   * DiscoveryMessageType identifies the purpose of a message exchanged during
   * peer discovery.
   *
   * It is used by:
   * - DiscoveryMessage
   * - DiscoveryEnvelope
   * - DiscoveryEncoder
   * - DiscoveryDecoder
   * - DiscoveryEngine
   *
   * Rules:
   * - Values must remain stable over time.
   * - Do not reorder existing values.
   * - Do not remove existing values once released.
   * - Add new values only at the end.
   */
  enum class DiscoveryMessageType : std::uint8_t
  {
    /**
     * @brief Unknown or invalid message type.
     */
    Unknown = 0,

    /**
     * @brief Periodic peer presence announcement.
     */
    Announce,

    /**
     * @brief Discovery probe request.
     */
    Probe,

    /**
     * @brief Direct reply to a probe.
     */
    Reply
  };

  /**
   * @brief Returns a stable string representation of a discovery message type.
   *
   * This is intended for logs, diagnostics, and text serialization.
   *
   * @param type Discovery message type.
   * @return Stable string representation.
   */
  [[nodiscard]] constexpr std::string_view
  to_string(DiscoveryMessageType type) noexcept
  {
    switch (type)
    {
    case DiscoveryMessageType::Unknown:
      return "unknown";

    case DiscoveryMessageType::Announce:
      return "announce";

    case DiscoveryMessageType::Probe:
      return "probe";

    case DiscoveryMessageType::Reply:
      return "reply";

    default:
      return "invalid";
    }
  }

  /**
   * @brief Returns true if the discovery message type is usable.
   *
   * Unknown is intentionally treated as invalid.
   *
   * @param type Discovery message type.
   * @return true for known usable message types.
   */
  [[nodiscard]] constexpr bool is_valid(DiscoveryMessageType type) noexcept
  {
    return type == DiscoveryMessageType::Announce ||
           type == DiscoveryMessageType::Probe ||
           type == DiscoveryMessageType::Reply;
  }

  /**
   * @brief Returns true if the message advertises local peer presence.
   *
   * @param type Discovery message type.
   * @return true for Announce.
   */
  [[nodiscard]] constexpr bool is_announcement(
      DiscoveryMessageType type) noexcept
  {
    return type == DiscoveryMessageType::Announce;
  }

  /**
   * @brief Returns true if the message is part of a request/reply discovery flow.
   *
   * @param type Discovery message type.
   * @return true for Probe and Reply.
   */
  [[nodiscard]] constexpr bool is_probe_flow(
      DiscoveryMessageType type) noexcept
  {
    return type == DiscoveryMessageType::Probe ||
           type == DiscoveryMessageType::Reply;
  }

} // namespace softadastra::discovery::types

#endif // SOFTADASTRA_DISCOVERY_MESSAGE_TYPE_HPP
