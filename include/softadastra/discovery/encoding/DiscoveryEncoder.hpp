/*
 * DiscoveryEncoder.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_ENCODER_HPP
#define SOFTADASTRA_DISCOVERY_ENCODER_HPP

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <softadastra/discovery/core/DiscoveryMessage.hpp>
#include <softadastra/discovery/utils/Datagram.hpp>

namespace softadastra::discovery::encoding
{
  namespace core = softadastra::discovery::core;
  namespace utils = softadastra::discovery::utils;

  class DiscoveryEncoder
  {
  public:
    /**
     * @brief Encode a discovery message payload
     *
     * Payload format:
     *   [uint8_t  type]
     *   [uint32_t from_size][from bytes]
     *   [uint32_t to_size][to bytes]
     *   [uint32_t correlation_size][correlation bytes]
     *   [uint32_t payload_size][payload bytes]
     */
    static std::vector<std::uint8_t> encode_message(const core::DiscoveryMessage &message)
    {
      const std::uint32_t from_size =
          static_cast<std::uint32_t>(message.from_node_id.size());

      const std::uint32_t to_size =
          static_cast<std::uint32_t>(message.to_node_id.size());

      const std::uint32_t correlation_size =
          static_cast<std::uint32_t>(message.correlation_id.size());

      const std::uint32_t payload_size =
          static_cast<std::uint32_t>(message.payload.size());

      const std::size_t total_size =
          sizeof(std::uint8_t) +
          sizeof(std::uint32_t) + from_size +
          sizeof(std::uint32_t) + to_size +
          sizeof(std::uint32_t) + correlation_size +
          sizeof(std::uint32_t) + payload_size;

      std::vector<std::uint8_t> buffer(total_size);

      std::size_t offset = 0;

      write(buffer, offset, static_cast<std::uint8_t>(message.type));
      write_string(buffer, offset, message.from_node_id);
      write_string(buffer, offset, message.to_node_id);
      write_string(buffer, offset, message.correlation_id);
      write_bytes(buffer, offset, message.payload);

      return buffer;
    }

    /**
     * @brief Wrap a discovery message into a Datagram structure
     */
    static utils::Datagram make_datagram(const core::DiscoveryMessage &message,
                                         const std::string &host,
                                         std::uint16_t port)
    {
      utils::Datagram datagram;
      datagram.host = host;
      datagram.port = port;
      datagram.payload = encode_message(message);
      return datagram;
    }

  private:
    template <typename T>
    static void write(std::vector<std::uint8_t> &buffer,
                      std::size_t &offset,
                      T value)
    {
      std::memcpy(buffer.data() + offset, &value, sizeof(T));
      offset += sizeof(T);
    }

    static void write_string(std::vector<std::uint8_t> &buffer,
                             std::size_t &offset,
                             const std::string &value)
    {
      const std::uint32_t size = static_cast<std::uint32_t>(value.size());
      write(buffer, offset, size);

      if (size > 0)
      {
        std::memcpy(buffer.data() + offset, value.data(), size);
        offset += size;
      }
    }

    static void write_bytes(std::vector<std::uint8_t> &buffer,
                            std::size_t &offset,
                            const std::vector<std::uint8_t> &value)
    {
      const std::uint32_t size = static_cast<std::uint32_t>(value.size());
      write(buffer, offset, size);

      if (size > 0)
      {
        std::memcpy(buffer.data() + offset, value.data(), size);
        offset += size;
      }
    }
  };

} // namespace softadastra::discovery::encoding

#endif
