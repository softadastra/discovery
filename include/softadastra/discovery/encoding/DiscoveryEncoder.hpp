/**
 *
 *  @file DiscoveryEncoder.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_ENCODER_HPP
#define SOFTADASTRA_DISCOVERY_ENCODER_HPP

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <softadastra/store/utils/Serializer.hpp>
#include <softadastra/discovery/core/DiscoveryMessage.hpp>
#include <softadastra/discovery/utils/Datagram.hpp>

namespace softadastra::discovery::encoding
{
  namespace core = softadastra::discovery::core;
  namespace utils = softadastra::discovery::utils;
  namespace store_utils = softadastra::store::utils;

  /**
   * @brief Encodes discovery messages into binary payloads.
   *
   * DiscoveryEncoder converts a DiscoveryMessage into a deterministic binary
   * representation ready to be sent inside a UDP datagram.
   *
   * Message payload format:
   *
   * @code
   * uint8  message_type
   * uint32 from_size
   * bytes  from_node_id
   * uint32 to_size
   * bytes  to_node_id
   * uint32 correlation_size
   * bytes  correlation_id
   * uint32 payload_size
   * bytes  payload
   * @endcode
   *
   * Integers are encoded through the shared Store serializer helpers to keep
   * binary formats stable across Softadastra modules.
   */
  class DiscoveryEncoder
  {
  public:
    /**
     * @brief Encodes a discovery message payload.
     *
     * Invalid messages return an empty buffer.
     *
     * @param message Discovery message.
     * @return Encoded message bytes.
     */
    [[nodiscard]] static std::vector<std::uint8_t>
    encode_message(const core::DiscoveryMessage &message)
    {
      if (!message.is_valid())
      {
        return {};
      }

      std::vector<std::uint8_t> buffer;

      buffer.reserve(
          sizeof(std::uint8_t) +
          sizeof(std::uint32_t) + message.from_node_id.size() +
          sizeof(std::uint32_t) + message.to_node_id.size() +
          sizeof(std::uint32_t) + message.correlation_id.size() +
          sizeof(std::uint32_t) + message.payload.size());

      buffer.push_back(static_cast<std::uint8_t>(message.type));

      append_string(buffer, message.from_node_id);
      append_string(buffer, message.to_node_id);
      append_string(buffer, message.correlation_id);
      append_bytes(buffer, message.payload);

      return buffer;
    }

    /**
     * @brief Wraps a discovery message into a Datagram.
     *
     * Invalid messages or invalid endpoints return an empty invalid datagram.
     *
     * @param message Discovery message.
     * @param host Destination host.
     * @param port Destination port.
     * @return Datagram ready for the UDP backend.
     */
    [[nodiscard]] static utils::Datagram
    make_datagram(
        const core::DiscoveryMessage &message,
        const std::string &host,
        std::uint16_t port)
    {
      if (host.empty() || port == 0)
      {
        return utils::Datagram{};
      }

      auto payload = encode_message(message);

      if (payload.empty())
      {
        return utils::Datagram{};
      }

      return utils::Datagram{
          host,
          port,
          std::move(payload)};
    }

  private:
    /**
     * @brief Appends a size-prefixed string.
     *
     * @param buffer Output buffer.
     * @param value String value.
     */
    static void append_string(
        std::vector<std::uint8_t> &buffer,
        const std::string &value)
    {
      store_utils::Serializer::append_u32(
          buffer,
          static_cast<std::uint32_t>(value.size()));

      buffer.insert(
          buffer.end(),
          value.begin(),
          value.end());
    }

    /**
     * @brief Appends size-prefixed bytes.
     *
     * @param buffer Output buffer.
     * @param value Byte payload.
     */
    static void append_bytes(
        std::vector<std::uint8_t> &buffer,
        const std::vector<std::uint8_t> &value)
    {
      store_utils::Serializer::append_u32(
          buffer,
          static_cast<std::uint32_t>(value.size()));

      buffer.insert(
          buffer.end(),
          value.begin(),
          value.end());
    }
  };

} // namespace softadastra::discovery::encoding

#endif // SOFTADASTRA_DISCOVERY_ENCODER_HPP
