/*
 * DiscoveryOptions.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_OPTIONS_HPP
#define SOFTADASTRA_DISCOVERY_OPTIONS_HPP

#include <cstdint>
#include <string>

namespace softadastra::discovery
{
  struct DiscoveryOptions
  {
    std::string node_id;
    std::string bind_host{"0.0.0.0"};
    std::uint16_t bind_port{9400};

    std::string broadcast_host{"255.255.255.255"};
    std::uint16_t broadcast_port{9400};

    std::string announce_host{"127.0.0.1"};
    std::uint16_t announce_port{0};

    std::uint64_t announce_interval_ms{3000};
    std::uint64_t peer_ttl_ms{15000};

    bool enable_broadcast{true};

    bool valid() const noexcept
    {
      return !node_id.empty() &&
             !bind_host.empty() &&
             bind_port != 0 &&
             !broadcast_host.empty() &&
             broadcast_port != 0 &&
             !announce_host.empty() &&
             announce_port != 0 &&
             announce_interval_ms > 0 &&
             peer_ttl_ms > 0;
    }
  };
}

#endif
