/*
 * IDiscoveryBackend.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_I_DISCOVERY_BACKEND_HPP
#define SOFTADASTRA_DISCOVERY_I_DISCOVERY_BACKEND_HPP

#include <optional>

#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>

namespace softadastra::discovery::backend
{
  namespace core = softadastra::discovery::core;

  /**
   * @brief Abstract discovery backend interface
   *
   * A backend is responsible only for low-level discovery message delivery.
   * It does not contain peer registry or transport integration logic.
   */
  class IDiscoveryBackend
  {
  public:
    virtual ~IDiscoveryBackend() = default;

    /**
     * @brief Start the backend
     */
    virtual bool start() = 0;

    /**
     * @brief Stop the backend
     */
    virtual void stop() = 0;

    /**
     * @brief Return whether the backend is running
     */
    virtual bool running() const noexcept = 0;

    /**
     * @brief Send one discovery envelope
     */
    virtual bool send(const core::DiscoveryEnvelope &envelope) = 0;

    /**
     * @brief Poll one received discovery envelope if available
     *
     * Returns std::nullopt when no message is available.
     */
    virtual std::optional<core::DiscoveryEnvelope> poll() = 0;
  };

} // namespace softadastra::discovery::backend

#endif
