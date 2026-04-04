/*
 * DiscoveryConfig.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_CONFIG_HPP
#define SOFTADASTRA_DISCOVERY_CONFIG_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace softadastra::discovery::core
{
  /**
   * @brief Runtime configuration for the discovery layer
   */
  struct DiscoveryConfig
  {
    /**
     * Local bind host
     */
    std::string bind_host{"0.0.0.0"};

    /**
     * Local bind port
     */
    std::uint16_t bind_port{0};

    /**
     * Broadcast host used for peer announcements
     */
    std::string broadcast_host{"255.255.255.255"};

    /**
     * Broadcast port used for peer announcements
     */
    std::uint16_t broadcast_port{0};

    /**
     * Logical local node identifier
     */
    std::string node_id;

    /**
     * Public host announced to other peers
     */
    std::string announce_host{"127.0.0.1"};

    /**
     * Public port announced to other peers
     */
    std::uint16_t announce_port{0};

    /**
     * Periodic announce interval in milliseconds
     */
    std::uint64_t announce_interval_ms{3000};

    /**
     * Peer expiration timeout in milliseconds
     */
    std::uint64_t peer_ttl_ms{15000};

    /**
     * Maximum allowed datagram payload size in bytes
     */
    std::size_t max_datagram_size{64 * 1024};

    /**
     * Enable broadcast announcements
     */
    bool enable_broadcast{true};

    /**
     * @brief Check whether the configuration is minimally valid
     */
    bool valid() const noexcept
    {
      return !bind_host.empty() &&
             bind_port != 0 &&
             !broadcast_host.empty() &&
             broadcast_port != 0 &&
             !node_id.empty() &&
             !announce_host.empty() &&
             announce_port != 0 &&
             announce_interval_ms > 0 &&
             peer_ttl_ms > 0 &&
             max_datagram_size > 0;
    }
  };

} // namespace softadastra::discovery::core

#endif
