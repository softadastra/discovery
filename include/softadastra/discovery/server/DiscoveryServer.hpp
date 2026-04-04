/*
 * DiscoveryServer.hpp
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
   * @brief Inbound discovery server
   *
   * Responsible for:
   * - starting and stopping the backend listener side
   * - polling received discovery envelopes from the backend
   *
   * This class does not contain peer registry or transport logic.
   * It is a thin server-side wrapper around the discovery backend.
   */
  class DiscoveryServer
  {
  public:
    explicit DiscoveryServer(backend::IDiscoveryBackend &backend)
        : backend_(backend)
    {
    }

    /**
     * @brief Start the server/backend
     */
    bool start()
    {
      return backend_.start();
    }

    /**
     * @brief Stop the server/backend
     */
    void stop()
    {
      backend_.stop();
    }

    /**
     * @brief Return whether the backend is currently running
     */
    bool running() const noexcept
    {
      return backend_.running();
    }

    /**
     * @brief Poll one received discovery envelope
     *
     * Returns std::nullopt when no inbound message is available.
     */
    std::optional<core::DiscoveryEnvelope> poll()
    {
      return backend_.poll();
    }

  private:
    backend::IDiscoveryBackend &backend_;
  };

} // namespace softadastra::discovery::server

#endif
