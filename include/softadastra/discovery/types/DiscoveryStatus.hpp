/*
 * DiscoveryStatus.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_STATUS_HPP
#define SOFTADASTRA_DISCOVERY_STATUS_HPP

#include <cstdint>

namespace softadastra::discovery::types
{
  /**
   * @brief Runtime state of the discovery engine
   */
  enum class DiscoveryStatus : std::uint8_t
  {
    Stopped = 0,
    Starting,
    Running,
    Stopping,
    Failed
  };

} // namespace softadastra::discovery::types

#endif
