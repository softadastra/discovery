/**
 *
 *  @file DiscoveryServer.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_SERVER_HPP
#define SOFTADASTRA_DISCOVERY_SERVER_HPP

#include <optional>

#include <softadastra/discovery/backend/IDiscoveryBackend.hpp>
#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>

namespace softadastra::discovery::server
{
  namespace backend = softadastra::discovery::backend;
  namespace core = softadastra::discovery::core;

  /**
   * @brief Thin inbound discovery server.
   *
   * DiscoveryServer is a small server-side wrapper around an
   * IDiscoveryBackend.
   *
   * It is responsible for:
   * - starting the backend listener
   * - stopping the backend listener
   * - polling received discovery envelopes
   *
   * It does not contain peer registry logic.
   * It does not contain transport integration logic.
   */
  class DiscoveryServer
  {
  public:
    /**
     * @brief Creates a discovery server from a backend.
     *
     * The backend is not owned by the server.
     *
     * @param discovery_backend Discovery backend reference.
     */
    explicit DiscoveryServer(
        backend::IDiscoveryBackend &discovery_backend) noexcept
        : backend_(discovery_backend)
    {
    }

    /**
     * @brief Starts the server/backend.
     *
     * @return true on success.
     */
    bool start()
    {
      return backend_.start();
    }

    /**
     * @brief Stops the server/backend.
     */
    void stop()
    {
      backend_.stop();
    }

    /**
     * @brief Returns whether the backend is currently running.
     *
     * @return true when running.
     */
    [[nodiscard]] bool is_running() const noexcept
    {
      return backend_.is_running();
    }

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
     * @brief Polls one received discovery envelope.
     *
     * Returns std::nullopt when no inbound message is available.
     *
     * @return Received envelope or std::nullopt.
     */
    [[nodiscard]] std::optional<core::DiscoveryEnvelope> poll()
    {
      return backend_.poll();
    }

    /**
     * @brief Returns the underlying backend.
     *
     * @return Backend reference.
     */
    [[nodiscard]] backend::IDiscoveryBackend &backend() noexcept
    {
      return backend_;
    }

    /**
     * @brief Returns the underlying backend.
     *
     * @return Backend const reference.
     */
    [[nodiscard]] const backend::IDiscoveryBackend &backend() const noexcept
    {
      return backend_;
    }

  private:
    backend::IDiscoveryBackend &backend_;
  };

} // namespace softadastra::discovery::server

#endif // SOFTADASTRA_DISCOVERY_SERVER_HPP
