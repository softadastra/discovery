/*
 * Peer.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_PEER_HPP
#define SOFTADASTRA_DISCOVERY_PEER_HPP

#include <cstdint>
#include <string>

namespace softadastra::discovery
{
  struct Peer
  {
    std::string node_id;
    std::string host;
    std::uint16_t port{0};
    std::uint64_t last_seen_at{0};

    bool valid() const noexcept
    {
      return !node_id.empty() &&
             !host.empty() &&
             port != 0;
    }
  };
}

#endif
