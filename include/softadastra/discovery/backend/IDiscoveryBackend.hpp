/**
 *
 *  @file IDiscoveryBackend.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_I_DISCOVERY_BACKEND_HPP
#define SOFTADASTRA_DISCOVERY_I_DISCOVERY_BACKEND_HPP

#include <optional>

#include <softadastra/core/Core.hpp>
#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>

namespace softadastra::discovery::backend
{
  namespace core = softadastra::discovery::core;
  namespace core_types = softadastra::core::types;

  /**
   * @brief Abstract discovery backend interface.
   *
   * IDiscoveryBackend defines the low-level discovery delivery contract.
   *
   * A backend is responsible only for discovery message delivery:
   * - start / stop
   * - send discovery envelope
   * - poll received discovery envelope
   *
   * It must not contain peer registry logic.
   * It must not contain transport integration logic.
   */
  class IDiscoveryBackend : public core_types::NonCopyable
  {
  public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~IDiscoveryBackend() = default;

    /**
     * @brief Move constructor.
     */
    IDiscoveryBackend(IDiscoveryBackend &&) noexcept = default;

    /**
     * @brief Move assignment.
     */
    IDiscoveryBackend &operator=(IDiscoveryBackend &&) noexcept = default;

    /**
     * @brief Starts the discovery backend.
     *
     * @return true on success.
     */
    virtual bool start() = 0;

    /**
     * @brief Stops the discovery backend.
     */
    virtual void stop() = 0;

    /**
     * @brief Returns whether the backend is running.
     *
     * @return true when running.
     */
    [[nodiscard]] virtual bool is_running() const noexcept = 0;

    /**
     * @brief Backward-compatible running alias.
     *
     * @return true when running.
     */
    [[nodiscard]] bool running() const noexcept
    {
      return is_running();
    }

    /**
     * @brief Sends one discovery envelope.
     *
     * @param envelope Discovery envelope.
     * @return true on success.
     */
    virtual bool send(const core::DiscoveryEnvelope &envelope) = 0;

    /**
     * @brief Polls one received discovery envelope if available.
     *
     * Returns std::nullopt when no message is available.
     *
     * @return Discovery envelope or std::nullopt.
     */
    [[nodiscard]] virtual std::optional<core::DiscoveryEnvelope> poll() = 0;

  protected:
    /**
     * @brief Protected default constructor.
     */
    IDiscoveryBackend() = default;
  };

} // namespace softadastra::discovery::backend

#endif // SOFTADASTRA_DISCOVERY_I_DISCOVERY_BACKEND_HPP
