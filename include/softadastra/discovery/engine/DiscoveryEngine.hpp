/*
 * DiscoveryEngine.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_ENGINE_HPP
#define SOFTADASTRA_DISCOVERY_ENGINE_HPP

#include <cstdint>
#include <ctime>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <softadastra/discovery/backend/IDiscoveryBackend.hpp>
#include <softadastra/discovery/client/DiscoveryClient.hpp>
#include <softadastra/discovery/core/DiscoveryAnnouncement.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/discovery/core/DiscoveryContext.hpp>
#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>
#include <softadastra/discovery/core/DiscoveryMessage.hpp>
#include <softadastra/discovery/encoding/DiscoveryDecoder.hpp>
#include <softadastra/discovery/encoding/DiscoveryEncoder.hpp>
#include <softadastra/discovery/peer/DiscoveredPeer.hpp>
#include <softadastra/discovery/peer/DiscoveryRegistry.hpp>
#include <softadastra/discovery/server/DiscoveryServer.hpp>
#include <softadastra/discovery/types/DiscoveryMessageType.hpp>
#include <softadastra/discovery/types/DiscoveryPeerState.hpp>
#include <softadastra/discovery/types/DiscoveryStatus.hpp>
#include <softadastra/transport/core/PeerInfo.hpp>

namespace softadastra::discovery::engine
{
  namespace discovery_backend = softadastra::discovery::backend;
  namespace discovery_client = softadastra::discovery::client;
  namespace discovery_core = softadastra::discovery::core;
  namespace discovery_peer = softadastra::discovery::peer;
  namespace discovery_server = softadastra::discovery::server;
  namespace discovery_types = softadastra::discovery::types;
  namespace transport_core = softadastra::transport::core;

  /**
   * @brief Orchestrates discovery send/receive operations
   *
   * Responsibilities:
   * - own the discovery backend wrappers
   * - manage discovered peer registry state
   * - broadcast local announcements
   * - poll inbound discovery messages
   * - register discovered peers into the transport layer
   */
  class DiscoveryEngine
  {
  public:
    explicit DiscoveryEngine(const discovery_core::DiscoveryContext &context,
                             discovery_backend::IDiscoveryBackend &backend)
        : context_(context),
          backend_(backend),
          client_(backend),
          server_(backend)
    {
      if (!context_.valid())
      {
        throw std::runtime_error("DiscoveryEngine: invalid DiscoveryContext");
      }
    }

    /**
     * @brief Start the discovery engine
     */
    bool start()
    {
      if (status_ == discovery_types::DiscoveryStatus::Running)
      {
        return true;
      }

      status_ = discovery_types::DiscoveryStatus::Starting;

      if (!server_.start())
      {
        status_ = discovery_types::DiscoveryStatus::Failed;
        return false;
      }

      last_announce_at_ = 0;
      status_ = discovery_types::DiscoveryStatus::Running;
      return true;
    }

    /**
     * @brief Stop the discovery engine
     */
    void stop()
    {
      status_ = discovery_types::DiscoveryStatus::Stopping;
      server_.stop();
      registry_.clear();
      status_ = discovery_types::DiscoveryStatus::Stopped;
    }

    /**
     * @brief Return current engine status
     */
    discovery_types::DiscoveryStatus status() const noexcept
    {
      return status_;
    }

    /**
     * @brief Return true if the engine is running
     */
    bool running() const noexcept
    {
      return status_ == discovery_types::DiscoveryStatus::Running &&
             backend_.running();
    }

    /**
     * @brief Broadcast one local announce immediately
     */
    bool announce_now()
    {
      if (!running())
      {
        return false;
      }

      const auto &config = context_.config_ref();

      discovery_core::DiscoveryEnvelope envelope;
      envelope.message = make_announce_message();
      envelope.to_host = config.broadcast_host;
      envelope.to_port = config.broadcast_port;
      envelope.timestamp = now_ms();
      envelope.retry_count = 0;
      envelope.last_attempt_at = 0;

      const bool sent = client_.send(envelope);
      if (sent)
      {
        last_announce_at_ = envelope.timestamp;
      }

      return sent;
    }

    /**
     * @brief Send one probe immediately
     */
    bool probe_now()
    {
      if (!running())
      {
        return false;
      }

      const auto &config = context_.config_ref();

      discovery_core::DiscoveryEnvelope envelope;
      envelope.message = make_probe_message();
      envelope.to_host = config.broadcast_host;
      envelope.to_port = config.broadcast_port;
      envelope.timestamp = now_ms();
      envelope.retry_count = 0;
      envelope.last_attempt_at = 0;

      return client_.send(envelope);
    }

    /**
     * @brief Poll and process one inbound discovery message
     *
     * Returns true if one inbound envelope was processed,
     * false if no message was available or processing failed.
     */
    bool poll_once()
    {
      if (!running())
      {
        return false;
      }

      maybe_announce();
      expire_peers();

      const auto inbound = server_.poll();
      if (!inbound.has_value())
      {
        return false;
      }

      if (!inbound->message.valid())
      {
        return false;
      }

      if (inbound->message.from_node_id == local_node_id())
      {
        return false;
      }

      switch (inbound->message.type)
      {
      case discovery_types::DiscoveryMessageType::Announce:
        return handle_announce(*inbound);

      case discovery_types::DiscoveryMessageType::Probe:
        return handle_probe(*inbound);

      case discovery_types::DiscoveryMessageType::Reply:
        return handle_reply(*inbound);

      default:
        return false;
      }
    }

    /**
     * @brief Poll up to max_messages inbound messages
     *
     * Returns the number of successfully processed messages.
     */
    std::size_t poll_many(std::size_t max_messages)
    {
      std::size_t processed = 0;

      for (std::size_t i = 0; i < max_messages; ++i)
      {
        if (!poll_once())
        {
          break;
        }

        ++processed;
      }

      return processed;
    }

    /**
     * @brief Read-only access to discovered peer registry
     */
    const discovery_peer::DiscoveryRegistry &peers() const noexcept
    {
      return registry_;
    }

    /**
     * @brief Return all alive discovered peers as transport peer infos
     */
    std::vector<transport_core::PeerInfo> alive_transport_peers() const
    {
      std::vector<transport_core::PeerInfo> result;

      for (const auto &peer : registry_.alive_peers())
      {
        if (peer.valid())
        {
          result.push_back(peer.to_peer_info());
        }
      }

      return result;
    }

    /**
     * @brief Read-only access to context
     */
    const discovery_core::DiscoveryContext &context() const noexcept
    {
      return context_;
    }

  private:
    bool handle_announce(const discovery_core::DiscoveryEnvelope &envelope)
    {
      const auto announcement = decode_announcement(envelope.message.payload);
      if (!announcement.has_value() || !announcement->valid())
      {
        return false;
      }

      if (announcement->node_id == local_node_id())
      {
        return false;
      }

      upsert_discovered_peer(*announcement);
      integrate_with_transport(*announcement);
      return true;
    }

    bool handle_probe(const discovery_core::DiscoveryEnvelope &envelope)
    {
      if (envelope.from_host.empty() || envelope.from_port == 0)
      {
        return false;
      }

      discovery_core::DiscoveryEnvelope reply;
      reply.message = make_reply_message(envelope.message.from_node_id,
                                         envelope.message.correlation_id);
      reply.to_host = envelope.from_host;
      reply.to_port = envelope.from_port;
      reply.timestamp = now_ms();
      reply.retry_count = 0;
      reply.last_attempt_at = 0;

      return client_.send(reply);
    }

    bool handle_reply(const discovery_core::DiscoveryEnvelope &envelope)
    {
      const auto announcement = decode_announcement(envelope.message.payload);
      if (!announcement.has_value() || !announcement->valid())
      {
        return false;
      }

      if (announcement->node_id == local_node_id())
      {
        return false;
      }

      upsert_discovered_peer(*announcement);
      integrate_with_transport(*announcement);
      return true;
    }

    void upsert_discovered_peer(const discovery_core::DiscoveryAnnouncement &announcement)
    {
      discovery_peer::DiscoveredPeer peer;
      peer.announcement = announcement;
      peer.state = discovery_types::DiscoveryPeerState::Alive;
      peer.last_seen_at = now_ms();
      peer.error_count = 0;

      registry_.upsert(std::move(peer));
    }

    void integrate_with_transport(const discovery_core::DiscoveryAnnouncement &announcement)
    {
      transport_core::PeerInfo peer;
      peer.node_id = announcement.node_id;
      peer.host = announcement.host;
      peer.port = announcement.port;

      if (!peer.valid())
      {
        return;
      }

      auto known = context_.transport_ref().peers().get(peer.node_id);
      if (known.has_value())
      {
        return;
      }

      context_.transport_ref().connect_peer(peer);
    }

    void maybe_announce()
    {
      const auto &config = context_.config_ref();
      const std::uint64_t now = now_ms();

      if (last_announce_at_ == 0 ||
          now - last_announce_at_ >= config.announce_interval_ms)
      {
        announce_now();
      }
    }

    void expire_peers()
    {
      const auto &config = context_.config_ref();
      const std::uint64_t now = now_ms();

      for (const auto &peer : registry_.all())
      {
        const std::uint64_t age =
            now > peer.last_seen_at ? (now - peer.last_seen_at) : 0;

        if (age >= config.peer_ttl_ms)
        {
          registry_.set_state(peer.announcement.node_id,
                              discovery_types::DiscoveryPeerState::Expired);
        }
        else if (age >= config.peer_ttl_ms / 2)
        {
          registry_.set_state(peer.announcement.node_id,
                              discovery_types::DiscoveryPeerState::Stale);
        }
        else
        {
          registry_.set_state(peer.announcement.node_id,
                              discovery_types::DiscoveryPeerState::Alive);
        }
      }
    }

    discovery_core::DiscoveryMessage make_announce_message() const
    {
      discovery_core::DiscoveryMessage message;
      message.type = discovery_types::DiscoveryMessageType::Announce;
      message.from_node_id = local_node_id();
      message.correlation_id = make_correlation_id("announce");

      const auto payload = encode_announcement(make_local_announcement());
      message.payload = payload;

      return message;
    }

    discovery_core::DiscoveryMessage make_probe_message() const
    {
      discovery_core::DiscoveryMessage message;
      message.type = discovery_types::DiscoveryMessageType::Probe;
      message.from_node_id = local_node_id();
      message.correlation_id = make_correlation_id("probe");
      return message;
    }

    discovery_core::DiscoveryMessage make_reply_message(
        const std::string &to_node_id,
        const std::string &correlation_id) const
    {
      discovery_core::DiscoveryMessage message;
      message.type = discovery_types::DiscoveryMessageType::Reply;
      message.from_node_id = local_node_id();
      message.to_node_id = to_node_id;
      message.correlation_id = correlation_id;
      message.payload = encode_announcement(make_local_announcement());
      return message;
    }

    discovery_core::DiscoveryAnnouncement make_local_announcement() const
    {
      discovery_core::DiscoveryAnnouncement announcement;
      announcement.node_id = local_node_id();
      announcement.host = context_.config_ref().announce_host;
      announcement.port = context_.config_ref().announce_port;
      announcement.timestamp = now_ms();
      return announcement;
    }

    static std::vector<std::uint8_t> encode_announcement(
        const discovery_core::DiscoveryAnnouncement &announcement)
    {
      const std::uint32_t node_id_size =
          static_cast<std::uint32_t>(announcement.node_id.size());
      const std::uint32_t host_size =
          static_cast<std::uint32_t>(announcement.host.size());

      const std::size_t total_size =
          sizeof(std::uint32_t) + node_id_size +
          sizeof(std::uint32_t) + host_size +
          sizeof(std::uint16_t) +
          sizeof(std::uint64_t);

      std::vector<std::uint8_t> buffer(total_size);
      std::size_t offset = 0;

      write_string(buffer, offset, announcement.node_id);
      write_string(buffer, offset, announcement.host);
      write_value(buffer, offset, announcement.port);
      write_value(buffer, offset, announcement.timestamp);

      return buffer;
    }

    static std::optional<discovery_core::DiscoveryAnnouncement> decode_announcement(
        const std::vector<std::uint8_t> &buffer)
    {
      std::size_t offset = 0;

      discovery_core::DiscoveryAnnouncement announcement;

      if (!read_string(buffer, offset, announcement.node_id))
      {
        return std::nullopt;
      }

      if (!read_string(buffer, offset, announcement.host))
      {
        return std::nullopt;
      }

      if (!read_value(buffer, offset, announcement.port))
      {
        return std::nullopt;
      }

      if (!read_value(buffer, offset, announcement.timestamp))
      {
        return std::nullopt;
      }

      if (offset != buffer.size())
      {
        return std::nullopt;
      }

      if (!announcement.valid())
      {
        return std::nullopt;
      }

      return announcement;
    }

    const std::string &local_node_id() const
    {
      return context_.config_ref().node_id;
    }

    std::string make_correlation_id(const std::string &prefix) const
    {
      return prefix + "-" + std::to_string(now_ms());
    }

    template <typename T>
    static void write_value(std::vector<std::uint8_t> &buffer,
                            std::size_t &offset,
                            T value)
    {
      std::memcpy(buffer.data() + offset, &value, sizeof(T));
      offset += sizeof(T);
    }

    static void write_string(std::vector<std::uint8_t> &buffer,
                             std::size_t &offset,
                             const std::string &value)
    {
      const std::uint32_t size = static_cast<std::uint32_t>(value.size());
      std::memcpy(buffer.data() + offset, &size, sizeof(std::uint32_t));
      offset += sizeof(std::uint32_t);

      if (size > 0)
      {
        std::memcpy(buffer.data() + offset, value.data(), size);
        offset += size;
      }
    }

    template <typename T>
    static bool read_value(const std::vector<std::uint8_t> &buffer,
                           std::size_t &offset,
                           T &value)
    {
      if (offset + sizeof(T) > buffer.size())
      {
        return false;
      }

      std::memcpy(&value, buffer.data() + offset, sizeof(T));
      offset += sizeof(T);
      return true;
    }

    static bool read_string(const std::vector<std::uint8_t> &buffer,
                            std::size_t &offset,
                            std::string &value)
    {
      if (offset + sizeof(std::uint32_t) > buffer.size())
      {
        return false;
      }

      std::uint32_t size = 0;
      std::memcpy(&size, buffer.data() + offset, sizeof(std::uint32_t));
      offset += sizeof(std::uint32_t);

      if (offset + size > buffer.size())
      {
        return false;
      }

      value.assign(reinterpret_cast<const char *>(buffer.data() + offset), size);
      offset += size;
      return true;
    }

    static std::uint64_t now_ms()
    {
      return static_cast<std::uint64_t>(std::time(nullptr)) * 1000ULL;
    }

  private:
    const discovery_core::DiscoveryContext &context_;
    discovery_backend::IDiscoveryBackend &backend_;

    discovery_client::DiscoveryClient client_;
    discovery_server::DiscoveryServer server_;
    discovery_peer::DiscoveryRegistry registry_;

    discovery_types::DiscoveryStatus status_{discovery_types::DiscoveryStatus::Stopped};
    std::uint64_t last_announce_at_{0};
  };

} // namespace softadastra::discovery::engine

#endif
