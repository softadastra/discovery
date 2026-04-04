/*
 * DiscoveryDecoder.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_DECODER_HPP
#define SOFTADASTRA_DISCOVERY_DECODER_HPP

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include <softadastra/discovery/core/DiscoveryMessage.hpp>
#include <softadastra/discovery/utils/Datagram.hpp>

namespace softadastra::discovery::encoding
{
  namespace core = softadastra::discovery::core;
  namespace types = softadastra::discovery::types;
  namespace utils = softadastra::discovery::utils;

  class DiscoveryDecoder
  {
  public:
    /**
     * @brief Decode a discovery message payload
     */
    static std::optional<core::DiscoveryMessage> decode_message(const std::uint8_t *data,
                                                                std::size_t size)
    {
      if (data == nullptr || size < minimum_message_size())
      {
        return std::nullopt;
      }

      std::size_t offset = 0;

      core::DiscoveryMessage message;

      std::uint8_t raw_type = 0;
      if (!read(data, size, offset, raw_type))
      {
        return std::nullopt;
      }

      message.type = static_cast<types::DiscoveryMessageType>(raw_type);

      if (!read_string(data, size, offset, message.from_node_id))
      {
        return std::nullopt;
      }

      if (!read_string(data, size, offset, message.to_node_id))
      {
        return std::nullopt;
      }

      if (!read_string(data, size, offset, message.correlation_id))
      {
        return std::nullopt;
      }

      if (!read_bytes(data, size, offset, message.payload))
      {
        return std::nullopt;
      }

      if (offset != size)
      {
        return std::nullopt;
      }

      if (!message.valid())
      {
        return std::nullopt;
      }

      return message;
    }

    /**
     * @brief Decode a discovery message from a raw byte vector
     */
    static std::optional<core::DiscoveryMessage> decode_message(
        const std::vector<std::uint8_t> &buffer)
    {
      if (buffer.empty())
      {
        return std::nullopt;
      }

      return decode_message(buffer.data(), buffer.size());
    }

    /**
     * @brief Decode a discovery message directly from a Datagram
     */
    static std::optional<core::DiscoveryMessage> decode_datagram(
        const utils::Datagram &datagram)
    {
      if (!datagram.valid())
      {
        return std::nullopt;
      }

      return decode_message(datagram.payload);
    }

  private:
    static constexpr std::size_t minimum_message_size()
    {
      return sizeof(std::uint8_t) +
             sizeof(std::uint32_t) +
             sizeof(std::uint32_t) +
             sizeof(std::uint32_t) +
             sizeof(std::uint32_t);
    }

    template <typename T>
    static bool read(const std::uint8_t *data,
                     std::size_t size,
                     std::size_t &offset,
                     T &value)
    {
      if (offset + sizeof(T) > size)
      {
        return false;
      }

      std::memcpy(&value, data + offset, sizeof(T));
      offset += sizeof(T);
      return true;
    }

    static bool read_string(const std::uint8_t *data,
                            std::size_t size,
                            std::size_t &offset,
                            std::string &out)
    {
      std::uint32_t length = 0;
      if (!read(data, size, offset, length))
      {
        return false;
      }

      if (offset + length > size)
      {
        return false;
      }

      out.assign(reinterpret_cast<const char *>(data + offset), length);
      offset += length;
      return true;
    }

    static bool read_bytes(const std::uint8_t *data,
                           std::size_t size,
                           std::size_t &offset,
                           std::vector<std::uint8_t> &out)
    {
      std::uint32_t length = 0;
      if (!read(data, size, offset, length))
      {
        return false;
      }

      if (offset + length > size)
      {
        return false;
      }

      out.resize(length);

      if (length > 0)
      {
        std::memcpy(out.data(), data + offset, length);
      }

      offset += length;
      return true;
    }
  };

} // namespace softadastra::discovery::encoding

#endif
