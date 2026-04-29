/**
 *
 *  @file DiscoveryMessage.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_MESSAGE_HPP
#define SOFTADASTRA_DISCOVERY_MESSAGE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <softadastra/discovery/types/DiscoveryMessageType.hpp>

namespace softadastra::discovery::core
{
  namespace types = softadastra::discovery::types;

  /**
   * @brief Logical discovery message exchanged between nodes.
   *
   * DiscoveryMessage represents a protocol-level discovery message before
   * datagram wrapping.
   *
   * It contains:
   * - semantic discovery message type
   * - sender node id
   * - optional recipient node id
   * - correlation id
   * - opaque payload bytes
   *
   * The discovery layer does not interpret arbitrary payload bytes directly.
   * Encoding and decoding helpers define how known payloads are represented.
   */
  struct DiscoveryMessage
  {
    /**
     * @brief Semantic discovery message type.
     */
    types::DiscoveryMessageType type{types::DiscoveryMessageType::Unknown};

    /**
     * @brief Sender node identifier.
     */
    std::string from_node_id{};

    /**
     * @brief Optional recipient node identifier.
     *
     * Empty means broadcast or unspecified recipient.
     */
    std::string to_node_id{};

    /**
     * @brief Correlation id used for probes and replies.
     */
    std::string correlation_id{};

    /**
     * @brief Opaque payload bytes.
     */
    std::vector<std::uint8_t> payload{};

    /**
     * @brief Creates an empty invalid discovery message.
     */
    DiscoveryMessage() = default;

    /**
     * @brief Creates a discovery message.
     *
     * @param message_type Semantic discovery message type.
     * @param from Sender node id.
     * @param payload_bytes Opaque payload bytes.
     */
    DiscoveryMessage(
        types::DiscoveryMessageType message_type,
        std::string from,
        std::vector<std::uint8_t> payload_bytes = {})
        : type(message_type),
          from_node_id(std::move(from)),
          payload(std::move(payload_bytes))
    {
    }

    /**
     * @brief Creates an announce message.
     *
     * @param from Sender node id.
     * @param payload_bytes Encoded announcement payload.
     * @return Discovery message.
     */
    [[nodiscard]] static DiscoveryMessage announce(
        std::string from,
        std::vector<std::uint8_t> payload_bytes)
    {
      return DiscoveryMessage{
          types::DiscoveryMessageType::Announce,
          std::move(from),
          std::move(payload_bytes)};
    }

    /**
     * @brief Creates a probe message.
     *
     * @param from Sender node id.
     * @return Discovery message.
     */
    [[nodiscard]] static DiscoveryMessage probe(std::string from)
    {
      return DiscoveryMessage{
          types::DiscoveryMessageType::Probe,
          std::move(from),
          {}};
    }

    /**
     * @brief Creates a reply message.
     *
     * @param from Sender node id.
     * @param payload_bytes Encoded announcement payload.
     * @return Discovery message.
     */
    [[nodiscard]] static DiscoveryMessage reply(
        std::string from,
        std::vector<std::uint8_t> payload_bytes)
    {
      return DiscoveryMessage{
          types::DiscoveryMessageType::Reply,
          std::move(from),
          std::move(payload_bytes)};
    }

    /**
     * @brief Returns true if the message has a recipient.
     *
     * @return true when to_node_id is not empty.
     */
    [[nodiscard]] bool has_recipient() const noexcept
    {
      return !to_node_id.empty();
    }

    /**
     * @brief Returns true if the message has a correlation id.
     *
     * @return true when correlation_id is not empty.
     */
    [[nodiscard]] bool has_correlation_id() const noexcept
    {
      return !correlation_id.empty();
    }

    /**
     * @brief Returns true if the message has payload bytes.
     *
     * @return true when payload is not empty.
     */
    [[nodiscard]] bool has_payload() const noexcept
    {
      return !payload.empty();
    }

    /**
     * @brief Returns the payload size in bytes.
     *
     * @return Payload byte count.
     */
    [[nodiscard]] std::size_t payload_size() const noexcept
    {
      return payload.size();
    }

    /**
     * @brief Returns true if the message is minimally valid.
     *
     * A valid message requires a known type and a sender node id.
     *
     * @return true when the message can be processed by discovery.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return types::is_valid(type) &&
             !from_node_id.empty();
    }

    /**
     * @brief Clears the message.
     */
    void clear() noexcept
    {
      type = types::DiscoveryMessageType::Unknown;
      from_node_id.clear();
      to_node_id.clear();
      correlation_id.clear();
      payload.clear();
    }
  };

} // namespace softadastra::discovery::core

#endif // SOFTADASTRA_DISCOVERY_MESSAGE_HPP
