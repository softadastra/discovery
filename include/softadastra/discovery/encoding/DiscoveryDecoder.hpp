/**
 *
 *  @file DiscoveryDecoder.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_DECODER_HPP
#define SOFTADASTRA_DISCOVERY_DECODER_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <softadastra/store/utils/Serializer.hpp>
#include <softadastra/discovery/core/DiscoveryMessage.hpp>
#include <softadastra/discovery/types/DiscoveryMessageType.hpp>
#include <softadastra/discovery/utils/Datagram.hpp>

namespace softadastra::discovery::encoding
{
  namespace core = softadastra::discovery::core;
  namespace types = softadastra::discovery::types;
  namespace utils = softadastra::discovery::utils;
  namespace store_utils = softadastra::store::utils;

  /**
   * @brief Decodes binary discovery payloads into messages.
   *
   * DiscoveryDecoder is the inverse of DiscoveryEncoder.
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
   * Integers are decoded using the shared Store serializer helpers to keep
   * binary formats deterministic across modules.
   */
  class DiscoveryDecoder
  {
  public:
    /**
     * @brief Decodes a discovery message payload.
     *
     * @param data Encoded message bytes.
     * @return DiscoveryMessage or std::nullopt on invalid input.
     */
    [[nodiscard]] static std::optional<core::DiscoveryMessage>
    decode_message(std::span<const std::uint8_t> data)
    {
      if (data.size() < minimum_message_size())
      {
        return std::nullopt;
      }

      std::size_t offset = 0;

      core::DiscoveryMessage message{};

      std::uint8_t raw_type = 0;

      if (!read_u8(data, offset, raw_type))
      {
        return std::nullopt;
      }

      message.type =
          static_cast<types::DiscoveryMessageType>(raw_type);

      if (!read_string(data, offset, message.from_node_id))
      {
        return std::nullopt;
      }

      if (!read_string(data, offset, message.to_node_id))
      {
        return std::nullopt;
      }

      if (!read_string(data, offset, message.correlation_id))
      {
        return std::nullopt;
      }

      if (!read_bytes(data, offset, message.payload))
      {
        return std::nullopt;
      }

      if (offset != data.size())
      {
        return std::nullopt;
      }

      if (!message.is_valid())
      {
        return std::nullopt;
      }

      return message;
    }

    /**
     * @brief Decodes a discovery message payload from raw pointer and size.
     *
     * @param data Encoded message bytes.
     * @param size Byte count.
     * @return DiscoveryMessage or std::nullopt on invalid input.
     */
    [[nodiscard]] static std::optional<core::DiscoveryMessage>
    decode_message(const std::uint8_t *data, std::size_t size)
    {
      if (data == nullptr)
      {
        return std::nullopt;
      }

      return decode_message(
          std::span<const std::uint8_t>(data, size));
    }

    /**
     * @brief Decodes a discovery message from a raw byte vector.
     *
     * @param buffer Encoded message bytes.
     * @return DiscoveryMessage or std::nullopt on invalid input.
     */
    [[nodiscard]] static std::optional<core::DiscoveryMessage>
    decode_message(const std::vector<std::uint8_t> &buffer)
    {
      if (buffer.empty())
      {
        return std::nullopt;
      }

      return decode_message(
          std::span<const std::uint8_t>(buffer.data(), buffer.size()));
    }

    /**
     * @brief Decodes a discovery message directly from a Datagram.
     *
     * @param datagram Datagram containing encoded discovery payload.
     * @return DiscoveryMessage or std::nullopt on invalid input.
     */
    [[nodiscard]] static std::optional<core::DiscoveryMessage>
    decode_datagram(const utils::Datagram &datagram)
    {
      if (!datagram.is_valid())
      {
        return std::nullopt;
      }

      return decode_message(datagram.payload);
    }

  private:
    /**
     * @brief Minimum possible discovery message size.
     */
    [[nodiscard]] static constexpr std::size_t minimum_message_size() noexcept
    {
      return sizeof(std::uint8_t) +
             sizeof(std::uint32_t) +
             sizeof(std::uint32_t) +
             sizeof(std::uint32_t) +
             sizeof(std::uint32_t);
    }

    /**
     * @brief Reads one uint8 value.
     */
    [[nodiscard]] static bool read_u8(
        std::span<const std::uint8_t> data,
        std::size_t &offset,
        std::uint8_t &value) noexcept
    {
      if (!store_utils::Serializer::can_read(
              data,
              offset,
              sizeof(std::uint8_t)))
      {
        return false;
      }

      value = data[offset];
      ++offset;
      return true;
    }

    /**
     * @brief Reads a size-prefixed string.
     */
    [[nodiscard]] static bool read_string(
        std::span<const std::uint8_t> data,
        std::size_t &offset,
        std::string &out)
    {
      std::uint32_t length = 0;

      if (!store_utils::Serializer::read_u32(
              data,
              offset,
              length))
      {
        return false;
      }

      if (!store_utils::Serializer::can_read(
              data,
              offset,
              length))
      {
        return false;
      }

      out.assign(
          reinterpret_cast<const char *>(data.data() + offset),
          length);

      offset += length;
      return true;
    }

    /**
     * @brief Reads size-prefixed bytes.
     */
    [[nodiscard]] static bool read_bytes(
        std::span<const std::uint8_t> data,
        std::size_t &offset,
        std::vector<std::uint8_t> &out)
    {
      std::uint32_t length = 0;

      if (!store_utils::Serializer::read_u32(
              data,
              offset,
              length))
      {
        return false;
      }

      if (!store_utils::Serializer::can_read(
              data,
              offset,
              length))
      {
        return false;
      }

      out.clear();
      out.reserve(length);

      out.insert(
          out.end(),
          data.begin() + static_cast<std::ptrdiff_t>(offset),
          data.begin() + static_cast<std::ptrdiff_t>(offset + length));

      offset += length;
      return true;
    }
  };

} // namespace softadastra::discovery::encoding

#endif // SOFTADASTRA_DISCOVERY_DECODER_HPP
