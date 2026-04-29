/**
 *
 *  @file DiscoveryStatus.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_STATUS_HPP
#define SOFTADASTRA_DISCOVERY_STATUS_HPP

#include <cstdint>
#include <string_view>

namespace softadastra::discovery::types
{
  /**
   * @brief Runtime status of the discovery engine.
   *
   * DiscoveryStatus describes the lifecycle of the discovery engine.
   *
   * It is used by:
   * - DiscoveryEngine
   * - DiscoveryClient
   * - DiscoveryServer
   * - backend implementations
   * - diagnostics and metrics
   *
   * Rules:
   * - Values must remain stable over time.
   * - Do not reorder existing values.
   * - Do not remove existing values once released.
   * - Add new values only at the end.
   */
  enum class DiscoveryStatus : std::uint8_t
  {
    /**
     * @brief Discovery engine is stopped.
     */
    Stopped = 0,

    /**
     * @brief Discovery engine is starting.
     */
    Starting,

    /**
     * @brief Discovery engine is running.
     */
    Running,

    /**
     * @brief Discovery engine is stopping.
     */
    Stopping,

    /**
     * @brief Discovery engine failed and needs inspection.
     */
    Failed
  };

  /**
   * @brief Returns a stable string representation of a discovery status.
   *
   * @param status Discovery status.
   * @return Stable string representation.
   */
  [[nodiscard]] constexpr std::string_view
  to_string(DiscoveryStatus status) noexcept
  {
    switch (status)
    {
    case DiscoveryStatus::Stopped:
      return "stopped";

    case DiscoveryStatus::Starting:
      return "starting";

    case DiscoveryStatus::Running:
      return "running";

    case DiscoveryStatus::Stopping:
      return "stopping";

    case DiscoveryStatus::Failed:
      return "failed";

    default:
      return "invalid";
    }
  }

  /**
   * @brief Returns true if the discovery status is known.
   *
   * @param status Discovery status.
   * @return true for all defined statuses.
   */
  [[nodiscard]] constexpr bool is_valid(DiscoveryStatus status) noexcept
  {
    return status == DiscoveryStatus::Stopped ||
           status == DiscoveryStatus::Starting ||
           status == DiscoveryStatus::Running ||
           status == DiscoveryStatus::Stopping ||
           status == DiscoveryStatus::Failed;
  }

  /**
   * @brief Returns true if discovery can send or receive discovery messages.
   *
   * @param status Discovery status.
   * @return true when running.
   */
  [[nodiscard]] constexpr bool is_running(DiscoveryStatus status) noexcept
  {
    return status == DiscoveryStatus::Running;
  }

  /**
   * @brief Returns true if discovery is transitioning.
   *
   * @param status Discovery status.
   * @return true for Starting and Stopping.
   */
  [[nodiscard]] constexpr bool is_transitioning(DiscoveryStatus status) noexcept
  {
    return status == DiscoveryStatus::Starting ||
           status == DiscoveryStatus::Stopping;
  }

  /**
   * @brief Returns true if discovery is stopped or failed.
   *
   * @param status Discovery status.
   * @return true for Stopped and Failed.
   */
  [[nodiscard]] constexpr bool is_terminal(DiscoveryStatus status) noexcept
  {
    return status == DiscoveryStatus::Stopped ||
           status == DiscoveryStatus::Failed;
  }

} // namespace softadastra::discovery::types

#endif // SOFTADASTRA_DISCOVERY_STATUS_HPP
