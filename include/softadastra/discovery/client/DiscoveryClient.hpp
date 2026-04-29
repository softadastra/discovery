/**
 *
 *  @file DiscoveryClient.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_CLIENT_HPP
#define SOFTADASTRA_DISCOVERY_CLIENT_HPP

#include <cstdint>
#include <string>
#include <utility>

#include <softadastra/discovery/backend/IDiscoveryBackend.hpp>
#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>
#include <softadastra/discovery/core/DiscoveryMessage.hpp>

namespace softadastra::discovery::client
{

  namespace backend = softadastra::discovery::backend;
  namespace core = softadastra::discovery::core;

  /**
   * @brief Thin outbound discovery client.
   *
   * DiscoveryClient is a small client-side wrapper around an
   * IDiscoveryBackend.
   *
   * It is responsible for:
   * - sending discovery envelopes
   * - building outbound envelopes from messages
   * - sending announce, probe, and reply messages
   *
   * It does not contain peer registry logic.
   * It does not contain transport integration logic.
   */
  class DiscoveryClient
  {
  public:
    /**
     * @brief Creates a discovery client from a backend.
     *
     * The backend is not owned by the client.
     *
     * @param discovery_backend Discovery backend reference.
     */
    explicit DiscoveryClient(
        backend::IDiscoveryBackend &discovery_backend) noexcept
        : backend_(discovery_backend)
    {
    }

    /**
     * @brief Sends one prebuilt discovery envelope.
     *
     * Invalid envelopes or missing destinations are rejected before calling
     * the backend.
     *
     * @param envelope Discovery envelope.
     * @return true on success.
     */
    bool send(core::DiscoveryEnvelope envelope)
    {
      if (!envelope.is_valid() ||
          !envelope.has_destination())
      {
        return false;
      }

      envelope.mark_attempt();

      return backend_.send(envelope);
    }

    /**
     * @brief Builds and sends one discovery message to host and port.
     *
     * @param host Destination host.
     * @param port Destination port.
     * @param message Discovery message.
     * @return true on success.
     */
    bool send_to(
        const std::string &host,
        std::uint16_t port,
        core::DiscoveryMessage message)
    {
      if (host.empty() ||
          port == 0 ||
          !message.is_valid())
      {
        return false;
      }

      core::DiscoveryEnvelope envelope{
          std::move(message),
          host,
          port};

      return send(std::move(envelope));
    }

    /**
     * @brief Sends an announcement message.
     *
     * @param host Destination host.
     * @param port Destination port.
     * @param from_node_id Sender node id.
     * @param payload Encoded announcement payload.
     * @return true on success.
     */
    bool send_announce(
        const std::string &host,
        std::uint16_t port,
        std::string from_node_id,
        std::vector<std::uint8_t> payload)
    {
      return send_to(
          host,
          port,
          core::DiscoveryMessage::announce(
              std::move(from_node_id),
              std::move(payload)));
    }

    /**
     * @brief Sends a discovery probe message.
     *
     * @param host Destination host.
     * @param port Destination port.
     * @param from_node_id Sender node id.
     * @return true on success.
     */
    bool send_probe(
        const std::string &host,
        std::uint16_t port,
        std::string from_node_id)
    {
      return send_to(
          host,
          port,
          core::DiscoveryMessage::probe(
              std::move(from_node_id)));
    }

    /**
     * @brief Sends a discovery reply message.
     *
     * @param host Destination host.
     * @param port Destination port.
     * @param from_node_id Sender node id.
     * @param payload Encoded announcement payload.
     * @return true on success.
     */
    bool send_reply(
        const std::string &host,
        std::uint16_t port,
        std::string from_node_id,
        std::vector<std::uint8_t> payload)
    {
      return send_to(
          host,
          port,
          core::DiscoveryMessage::reply(
              std::move(from_node_id),
              std::move(payload)));
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

  private:
    backend::IDiscoveryBackend &backend_;
  };

} // namespace softadastra::discovery::client

#endif // SOFTADASTRA_DISCOVERY_CLIENT_HPP
