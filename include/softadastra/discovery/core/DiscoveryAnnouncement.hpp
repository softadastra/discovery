/*
 * DiscoveryAnnouncement.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_ANNOUNCEMENT_HPP
#define SOFTADASTRA_DISCOVERY_ANNOUNCEMENT_HPP

#include <cstdint>
#include <string>

namespace softadastra::discovery::core
{
  /**
   * @brief Peer presence announcement payload
   *
   * This is the minimal identity and reachability information
   * advertised by one node to other peers.
   */
  struct DiscoveryAnnouncement
  {
    /**
     * Logical node identifier
     */
    std::string node_id;

    /**
     * Announced host or IP address
     */
    std::string host{"127.0.0.1"};

    /**
     * Announced listening port
     */
    std::uint16_t port{0};

    /**
     * Announcement timestamp in milliseconds
     */
    std::uint64_t timestamp{0};

    /**
     * @brief Check whether this announcement is usable
     */
    bool valid() const noexcept
    {
      return !node_id.empty() &&
             !host.empty() &&
             port != 0;
    }
  };

} // namespace softadastra::discovery::core

#endif
