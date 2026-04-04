/*
 * DiscoveryClient.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_CLIENT_HPP
#define SOFTADASTRA_DISCOVERY_CLIENT_HPP

#include <softadastra/discovery/backend/IDiscoveryBackend.hpp>
#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>
#include <softadastra/discovery/core/DiscoveryMessage.hpp>

namespace softadastra::discovery::client
{
  namespace backend = softadastra::discovery::backend;
  namespace core = softadastra::discovery::core;

  /**
   * @brief Outbound discovery client
   *
   * Responsible for:
   * - sending discovery messages
   * - broadcasting announcements
   * - replying to probes
   *
   * This class does not contain registry or transport logic.
   * It is a thin client-side wrapper around the discovery backend.
   */
  class DiscoveryClient
  {
  public:
    explicit DiscoveryClient(backend::IDiscoveryBackend &backend)
        : backend_(backend)
    {
    }

    /**
     * @brief Send one prebuilt discovery envelope
     */
    bool send(const core::DiscoveryEnvelope &envelope)
    {
      return backend_.send(envelope);
    }

    /**
     * @brief Build and send one discovery message to host:port
     */
    bool send_to(const std::string &host,
                 std::uint16_t port,
                 const core::DiscoveryMessage &message,
                 std::uint64_t now_ms = 0)
    {
      core::DiscoveryEnvelope envelope;
      envelope.message = message;
      envelope.to_host = host;
      envelope.to_port = port;
      envelope.timestamp = now_ms;
      envelope.retry_count = 0;
      envelope.last_attempt_at = 0;

      return backend_.send(envelope);
    }

    /**
     * @brief Return whether the backend is currently running
     */
    bool running() const noexcept
    {
      return backend_.running();
    }

  private:
    backend::IDiscoveryBackend &backend_;
  };

} // namespace softadastra::discovery::client

#endif
