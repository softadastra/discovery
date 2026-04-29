/**
 *
 *  @file DiscoveryEnvelope.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_ENVELOPE_HPP
#define SOFTADASTRA_DISCOVERY_ENVELOPE_HPP

#include <cstdint>
#include <string>
#include <utility>

#include <softadastra/core/Core.hpp>
#include <softadastra/discovery/core/DiscoveryMessage.hpp>

namespace softadastra::discovery::core
{
  namespace core_time = softadastra::core::time;

  /**
   * @brief Runtime wrapper around a discovery message.
   *
   * DiscoveryEnvelope adds network metadata and runtime bookkeeping around a
   * DiscoveryMessage.
   *
   * It tracks:
   * - source host
   * - source port
   * - destination host
   * - destination port
   * - creation timestamp
   * - retry count
   * - last attempt timestamp
   *
   * It is used by:
   * - DiscoveryClient
   * - DiscoveryServer
   * - DiscoveryEngine
   * - discovery backends
   */
  struct DiscoveryEnvelope
  {
    /**
     * @brief Discovery message being transported.
     */
    DiscoveryMessage message{};

    /**
     * @brief Source host from which the datagram was received.
     */
    std::string from_host{};

    /**
     * @brief Source port from which the datagram was received.
     */
    std::uint16_t from_port{0};

    /**
     * @brief Destination host to which the datagram should be sent.
     */
    std::string to_host{};

    /**
     * @brief Destination port to which the datagram should be sent.
     */
    std::uint16_t to_port{0};

    /**
     * @brief Envelope creation timestamp.
     */
    core_time::Timestamp timestamp{};

    /**
     * @brief Number of send attempts.
     */
    std::uint32_t retry_count{0};

    /**
     * @brief Last send attempt timestamp.
     */
    core_time::Timestamp last_attempt_at{};

    /**
     * @brief Creates an empty invalid envelope.
     */
    DiscoveryEnvelope() = default;

    /**
     * @brief Creates an envelope from a discovery message.
     *
     * @param discovery_message Discovery message.
     */
    explicit DiscoveryEnvelope(DiscoveryMessage discovery_message)
        : message(std::move(discovery_message)),
          timestamp(core_time::Timestamp::now())
    {
    }

    /**
     * @brief Creates an outbound envelope.
     *
     * @param discovery_message Discovery message.
     * @param destination_host Destination host.
     * @param destination_port Destination port.
     */
    DiscoveryEnvelope(
        DiscoveryMessage discovery_message,
        std::string destination_host,
        std::uint16_t destination_port)
        : message(std::move(discovery_message)),
          to_host(std::move(destination_host)),
          to_port(destination_port),
          timestamp(core_time::Timestamp::now())
    {
    }

    /**
     * @brief Returns true if the envelope has source metadata.
     *
     * @return true when source host and port are present.
     */
    [[nodiscard]] bool has_source() const noexcept
    {
      return !from_host.empty() &&
             from_port != 0;
    }

    /**
     * @brief Returns true if the envelope has destination metadata.
     *
     * @return true when destination host and port are present.
     */
    [[nodiscard]] bool has_destination() const noexcept
    {
      return !to_host.empty() &&
             to_port != 0;
    }

    /**
     * @brief Returns true if this envelope has been attempted at least once.
     *
     * @return true when retry_count is greater than zero.
     */
    [[nodiscard]] bool attempted() const noexcept
    {
      return retry_count > 0;
    }

    /**
     * @brief Marks a send attempt.
     *
     * This increments retry_count and updates last_attempt_at.
     */
    void mark_attempt() noexcept
    {
      ++retry_count;
      last_attempt_at = core_time::Timestamp::now();
    }

    /**
     * @brief Returns true if the envelope is usable.
     *
     * A valid envelope requires a valid discovery message.
     * Destination metadata is required only for outbound sends.
     *
     * @return true when message and timestamp are valid.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return message.is_valid() &&
             timestamp.is_valid();
    }

    /**
     * @brief Clears the envelope.
     */
    void clear() noexcept
    {
      message.clear();
      from_host.clear();
      from_port = 0;
      to_host.clear();
      to_port = 0;
      timestamp = core_time::Timestamp{};
      retry_count = 0;
      last_attempt_at = core_time::Timestamp{};
    }
  };

} // namespace softadastra::discovery::core

#endif // SOFTADASTRA_DISCOVERY_ENVELOPE_HPP
