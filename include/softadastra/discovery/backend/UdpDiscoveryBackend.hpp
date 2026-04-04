/*
 * UdpDiscoveryBackend.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_UDP_DISCOVERY_BACKEND_HPP
#define SOFTADASTRA_DISCOVERY_UDP_DISCOVERY_BACKEND_HPP

#include <optional>
#include <string>

#include <softadastra/discovery/backend/IDiscoveryBackend.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>
#include <softadastra/discovery/core/DiscoveryMessage.hpp>
#include <softadastra/discovery/encoding/DiscoveryDecoder.hpp>
#include <softadastra/discovery/encoding/DiscoveryEncoder.hpp>
#include <softadastra/discovery/platform/linux/UdpSocket.hpp>

namespace softadastra::discovery::backend
{
  namespace core = softadastra::discovery::core;
  namespace encoding = softadastra::discovery::encoding;
  namespace platform = softadastra::discovery::platform::os_linux;
  namespace utils = softadastra::discovery::utils;

  /**
   * @brief UDP discovery backend
   *
   * Responsible for:
   * - binding one UDP socket
   * - broadcasting discovery messages
   * - receiving discovery datagrams
   * - decoding them into discovery envelopes
   */
  class UdpDiscoveryBackend : public IDiscoveryBackend
  {
  public:
    explicit UdpDiscoveryBackend(const core::DiscoveryConfig &config)
        : config_(config)
    {
    }

    bool start() override
    {
      if (!config_.valid())
      {
        return false;
      }

      if (!socket_.open())
      {
        return false;
      }

      socket_.set_reuse_addr(true);
      socket_.set_broadcast(config_.enable_broadcast);
      socket_.set_recv_timeout_ms(config_.announce_interval_ms);
      socket_.set_send_timeout_ms(config_.announce_interval_ms);

      if (!socket_.bind(config_.bind_host, config_.bind_port))
      {
        socket_.close();
        return false;
      }

      running_ = true;
      return true;
    }

    void stop() override
    {
      running_ = false;
      socket_.close();
    }

    bool running() const noexcept override
    {
      return running_;
    }

    bool send(const core::DiscoveryEnvelope &envelope) override
    {
      if (!running_ || !envelope.valid())
      {
        return false;
      }

      if (envelope.to_host.empty() || envelope.to_port == 0)
      {
        return false;
      }

      const auto datagram = encoding::DiscoveryEncoder::make_datagram(
          envelope.message,
          envelope.to_host,
          envelope.to_port);

      const std::size_t sent = socket_.send_datagram(datagram);
      return sent == datagram.payload.size();
    }

    std::optional<core::DiscoveryEnvelope> poll() override
    {
      if (!running_)
      {
        return std::nullopt;
      }

      const auto datagram = socket_.recv_datagram(config_.max_datagram_size);
      if (!datagram.valid())
      {
        return std::nullopt;
      }

      const auto message = encoding::DiscoveryDecoder::decode_datagram(datagram);
      if (!message.has_value())
      {
        return std::nullopt;
      }

      core::DiscoveryEnvelope envelope;
      envelope.message = *message;
      envelope.from_host = datagram.host;
      envelope.from_port = datagram.port;
      envelope.timestamp = 0;
      envelope.retry_count = 0;
      envelope.last_attempt_at = 0;

      return envelope;
    }

  private:
    core::DiscoveryConfig config_;
    bool running_{false};
    platform::UdpSocket socket_;
  };

} // namespace softadastra::discovery::backend

#endif
