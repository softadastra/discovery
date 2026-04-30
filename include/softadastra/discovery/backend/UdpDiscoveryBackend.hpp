/**
 *
 *  @file UdpDiscoveryBackend.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_UDP_DISCOVERY_BACKEND_HPP
#define SOFTADASTRA_DISCOVERY_UDP_DISCOVERY_BACKEND_HPP

#include <cstddef>
#include <optional>
#include <utility>

#include <softadastra/core/Core.hpp>
#include <softadastra/discovery/backend/IDiscoveryBackend.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>
#include <softadastra/discovery/encoding/DiscoveryDecoder.hpp>
#include <softadastra/discovery/encoding/DiscoveryEncoder.hpp>
#include <softadastra/discovery/platform/linux/UdpSocket.hpp>

namespace softadastra::discovery::backend
{
  namespace core = softadastra::discovery::core;
  namespace encoding = softadastra::discovery::encoding;
  namespace platform = softadastra::discovery::platform::os_linux;
  namespace core_time = softadastra::core::time;

  /**
   * @brief UDP discovery backend.
   *
   * UdpDiscoveryBackend is the default low-level discovery backend.
   *
   * It is responsible for:
   * - opening one UDP socket
   * - binding to the configured address
   * - sending encoded discovery messages
   * - receiving discovery datagrams
   * - decoding datagrams into discovery envelopes
   *
   * It does not own peer registry logic.
   * It does not connect peers through transport directly.
   */
  class UdpDiscoveryBackend : public IDiscoveryBackend
  {
  public:
    /**
     * @brief Creates a UDP discovery backend.
     *
     * @param config Discovery configuration.
     */
    explicit UdpDiscoveryBackend(core::DiscoveryConfig config)
        : config_(std::move(config))
    {
    }

    /**
     * @brief Stops the backend on destruction.
     */
    ~UdpDiscoveryBackend() override
    {
      stop();
    }

    /**
     * @brief Starts the UDP backend.
     *
     * @return true on success.
     */
    bool start() override
    {
      if (running_)
      {
        return true;
      }

      if (!config_.is_valid())
      {
        return false;
      }

      if (!socket_.open())
      {
        return false;
      }

      if (!configure_socket())
      {
        socket_.close();
        return false;
      }

      if (!socket_.bind(config_.bind_host, config_.bind_port))
      {
        socket_.close();
        return false;
      }

      running_ = true;
      return true;
    }

    /**
     * @brief Stops the UDP backend.
     */
    void stop() override
    {
      running_ = false;
      socket_.close();
    }

    /**
     * @brief Returns whether the backend is running.
     *
     * @return true when running.
     */
    [[nodiscard]] bool is_running() const noexcept override
    {
      return running_;
    }

    /**
     * @brief Sends one discovery envelope.
     *
     * @param envelope Discovery envelope.
     * @return true when the full datagram payload was sent.
     */
    bool send(const core::DiscoveryEnvelope &envelope) override
    {
      if (!running_ ||
          !envelope.is_valid() ||
          !envelope.has_destination())
      {
        return false;
      }

      const auto datagram =
          encoding::DiscoveryEncoder::make_datagram(
              envelope.message,
              envelope.to_host,
              envelope.to_port);

      if (!datagram.is_valid())
      {
        return false;
      }

      const std::size_t sent =
          socket_.send_datagram(datagram);

      return sent == datagram.payload.size();
    }

    /**
     * @brief Polls one inbound discovery envelope if available.
     *
     * @return Discovery envelope or std::nullopt.
     */
    [[nodiscard]] std::optional<core::DiscoveryEnvelope> poll() override
    {
      if (!running_)
      {
        return std::nullopt;
      }

      const auto datagram =
          socket_.recv_datagram(config_.max_datagram_size);

      if (!datagram.is_valid())
      {
        return std::nullopt;
      }

      const auto message =
          encoding::DiscoveryDecoder::decode_datagram(datagram);

      if (!message.has_value())
      {
        return std::nullopt;
      }

      core::DiscoveryEnvelope envelope{*message};

      envelope.from_host = datagram.host;
      envelope.from_port = datagram.port;
      envelope.timestamp = core_time::Timestamp::now();

      return envelope;
    }

    /**
     * @brief Returns the active discovery configuration.
     *
     * @return Discovery configuration.
     */
    [[nodiscard]] const core::DiscoveryConfig &config() const noexcept
    {
      return config_;
    }

  private:
    [[nodiscard]] bool configure_socket()
    {
      if (!socket_.set_reuse_addr(true))
      {
        return false;
      }

      if (!socket_.set_broadcast(config_.enable_broadcast))
      {
        return false;
      }

      if (!socket_.set_recv_timeout(config_.announce_interval))
      {
        return false;
      }

      if (!socket_.set_send_timeout(config_.announce_interval))
      {
        return false;
      }

      return true;
    }

  private:
    core::DiscoveryConfig config_{};
    bool running_{false};
    platform::UdpSocket socket_{};
  };

} // namespace softadastra::discovery::backend

#endif // SOFTADASTRA_DISCOVERY_UDP_DISCOVERY_BACKEND_HPP
