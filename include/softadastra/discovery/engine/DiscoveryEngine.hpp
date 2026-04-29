/**
 *
 *  @file DiscoveryEngine.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_ENGINE_HPP
#define SOFTADASTRA_DISCOVERY_ENGINE_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <softadastra/core/Core.hpp>
#include <softadastra/store/utils/Serializer.hpp>
#include <softadastra/discovery/backend/IDiscoveryBackend.hpp>
#include <softadastra/discovery/client/DiscoveryClient.hpp>
#include <softadastra/discovery/core/DiscoveryAnnouncement.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/discovery/core/DiscoveryContext.hpp>
#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>
#include <softadastra/discovery/core/DiscoveryMessage.hpp>
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

  namespace core_time = softadastra::core::time;
  namespace store_utils = softadastra::store::utils;

  /**
   * @brief Orchestrates discovery send and receive operations.
   *
   * DiscoveryEngine is the high-level discovery facade.
   *
   * It coordinates:
   * - backend lifecycle
   * - outbound announcements
   * - outbound probes
   * - inbound discovery polling
   * - discovered peer registry updates
   * - transport peer integration
   *
   * It does not own transport delivery logic.
   * It only discovers peers and hands usable peer information to TransportEngine.
   */
  class DiscoveryEngine : public softadastra::core::types::NonCopyable
  {
  public:
    /**
     * @brief Creates a discovery engine.
     *
     * The context and backend are not owned by this engine.
     *
     * @param context Discovery context.
     * @param backend Discovery backend.
     */
    DiscoveryEngine(
        const discovery_core::DiscoveryContext &context,
        discovery_backend::IDiscoveryBackend &backend) noexcept
        : context_(context),
          backend_(backend),
          client_(backend),
          server_(backend)
    {
    }

    /**
     * @brief Stops the discovery engine on destruction.
     */
    ~DiscoveryEngine()
    {
      stop();
    }

    /**
     * @brief Move constructor.
     */
    DiscoveryEngine(DiscoveryEngine &&) noexcept = default;

    /**
     * @brief Move assignment.
     */
    DiscoveryEngine &operator=(DiscoveryEngine &&) noexcept = default;

    /**
     * @brief Starts the discovery engine.
     *
     * @return true on success.
     */
    bool start()
    {
      if (discovery_types::is_running(status_))
      {
        return true;
      }

      if (!context_.is_valid())
      {
        status_ = discovery_types::DiscoveryStatus::Failed;
        return false;
      }

      status_ = discovery_types::DiscoveryStatus::Starting;

      if (!server_.start())
      {
        status_ = discovery_types::DiscoveryStatus::Failed;
        return false;
      }

      last_announce_at_ = core_time::Timestamp{};
      status_ = discovery_types::DiscoveryStatus::Running;

      return true;
    }

    /**
     * @brief Stops the discovery engine.
     */
    void stop()
    {
      if (status_ == discovery_types::DiscoveryStatus::Stopped)
      {
        return;
      }

      status_ = discovery_types::DiscoveryStatus::Stopping;

      server_.stop();
      registry_.clear();

      last_announce_at_ = core_time::Timestamp{};
      status_ = discovery_types::DiscoveryStatus::Stopped;
    }

    /**
     * @brief Returns current engine status.
     *
     * @return Discovery status.
     */
    [[nodiscard]] discovery_types::DiscoveryStatus status() const noexcept
    {
      return status_;
    }

    /**
     * @brief Returns true if the discovery engine is running.
     *
     * @return true when status and backend are running.
     */
    [[nodiscard]] bool is_running() const noexcept
    {
      return discovery_types::is_running(status_) &&
             backend_.is_running();
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
     * @brief Broadcasts one local announcement immediately.
     *
     * @return true if the announcement was sent.
     */
    bool announce_now()
    {
      if (!is_running())
      {
        return false;
      }

      auto config_result = context_.config_checked();

      if (config_result.is_err())
      {
        return false;
      }

      const auto &config = *config_result.value();

      auto envelope =
          make_outbound_envelope(
              make_announce_message(),
              config.broadcast_host,
              config.broadcast_port);

      const bool sent =
          client_.send(std::move(envelope));

      if (sent)
      {
        last_announce_at_ = core_time::Timestamp::now();
      }

      return sent;
    }

    /**
     * @brief Sends one discovery probe immediately.
     *
     * @return true if the probe was sent.
     */
    bool probe_now()
    {
      if (!is_running())
      {
        return false;
      }

      auto config_result = context_.config_checked();

      if (config_result.is_err())
      {
        return false;
      }

      const auto &config = *config_result.value();

      auto envelope =
          make_outbound_envelope(
              make_probe_message(),
              config.broadcast_host,
              config.broadcast_port);

      return client_.send(std::move(envelope));
    }

    /**
     * @brief Polls and processes one inbound discovery message.
     *
     * @return true if one inbound envelope was processed.
     */
    bool poll_once()
    {
      if (!is_running())
      {
        return false;
      }

      maybe_announce();
      refresh_peer_states();

      const auto inbound = server_.poll();

      if (!inbound.has_value())
      {
        return false;
      }

      if (!inbound->message.is_valid())
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
     * @brief Polls up to max_messages inbound messages.
     *
     * @param max_messages Maximum number of messages to process.
     * @return Number of successfully processed messages.
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
     * @brief Returns read-only access to discovered peer registry.
     *
     * @return Discovery registry.
     */
    [[nodiscard]] const discovery_peer::DiscoveryRegistry &
    peers() const noexcept
    {
      return registry_;
    }

    /**
     * @brief Returns all available discovered peers as transport peer info.
     *
     * Expired peers are skipped.
     *
     * @return Transport peers.
     */
    [[nodiscard]] std::vector<transport_core::PeerInfo>
    available_transport_peers() const
    {
      return registry_.transport_peers();
    }

    /**
     * @brief Returns all alive discovered peers as transport peer info.
     *
     * @return Transport peers.
     */
    [[nodiscard]] std::vector<transport_core::PeerInfo>
    alive_transport_peers() const
    {
      std::vector<transport_core::PeerInfo> result;

      for (const auto &peer : registry_.alive_peers())
      {
        auto info = peer.to_peer_info();

        if (info.is_valid())
        {
          result.push_back(std::move(info));
        }
      }

      return result;
    }

    /**
     * @brief Returns read-only access to context.
     *
     * @return Discovery context.
     */
    [[nodiscard]] const discovery_core::DiscoveryContext &
    context() const noexcept
    {
      return context_;
    }

  private:
    /**
     * @brief Handles one inbound announcement.
     */
    bool handle_announce(
        const discovery_core::DiscoveryEnvelope &envelope)
    {
      const auto announcement =
          decode_announcement(envelope.message.payload);

      if (!announcement.has_value() ||
          !announcement->is_valid())
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

    /**
     * @brief Handles one inbound probe.
     */
    bool handle_probe(
        const discovery_core::DiscoveryEnvelope &envelope)
    {
      if (!envelope.has_source())
      {
        return false;
      }

      auto reply =
          make_reply_message(
              envelope.message.from_node_id,
              envelope.message.correlation_id);

      auto outbound =
          make_outbound_envelope(
              std::move(reply),
              envelope.from_host,
              envelope.from_port);

      return client_.send(std::move(outbound));
    }

    /**
     * @brief Handles one inbound reply.
     */
    bool handle_reply(
        const discovery_core::DiscoveryEnvelope &envelope)
    {
      const auto announcement =
          decode_announcement(envelope.message.payload);

      if (!announcement.has_value() ||
          !announcement->is_valid())
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

    /**
     * @brief Inserts or refreshes a discovered peer.
     */
    void upsert_discovered_peer(
        discovery_core::DiscoveryAnnouncement announcement)
    {
      registry_.upsert_announcement(std::move(announcement));
    }

    /**
     * @brief Hands a discovered peer to the transport engine.
     */
    void integrate_with_transport(
        const discovery_core::DiscoveryAnnouncement &announcement)
    {
      transport_core::PeerInfo peer{
          announcement.node_id,
          announcement.host,
          announcement.port};

      if (!peer.is_valid())
      {
        return;
      }

      auto transport_result = context_.transport_checked();

      if (transport_result.is_err())
      {
        return;
      }

      auto *transport = transport_result.value();

      if (transport->peers().contains(peer.node_id))
      {
        return;
      }

      transport->connect_peer(peer);
    }

    /**
     * @brief Announces when the configured interval has elapsed.
     */
    void maybe_announce()
    {
      auto config_result = context_.config_checked();

      if (config_result.is_err())
      {
        return;
      }

      const auto &config = *config_result.value();
      const auto now = core_time::Timestamp::now();

      if (!last_announce_at_.is_valid() ||
          now.millis() - last_announce_at_.millis() >=
              config.announce_interval.millis())
      {
        announce_now();
      }
    }

    /**
     * @brief Refreshes stale and expired peer states.
     */
    void refresh_peer_states()
    {
      auto config_result = context_.config_checked();

      if (config_result.is_err())
      {
        return;
      }

      const auto &config = *config_result.value();

      registry_.refresh_states(
          core_time::Timestamp::now(),
          core_time::Duration::from_millis(
              config.peer_ttl.millis() / 2),
          config.peer_ttl);
    }

    /**
     * @brief Builds an announce message.
     */
    [[nodiscard]] discovery_core::DiscoveryMessage
    make_announce_message() const
    {
      auto payload =
          encode_announcement(
              make_local_announcement());

      auto message =
          discovery_core::DiscoveryMessage::announce(
              local_node_id(),
              std::move(payload));

      message.correlation_id =
          make_correlation_id("announce");

      return message;
    }

    /**
     * @brief Builds a probe message.
     */
    [[nodiscard]] discovery_core::DiscoveryMessage
    make_probe_message() const
    {
      auto message =
          discovery_core::DiscoveryMessage::probe(
              local_node_id());

      message.correlation_id =
          make_correlation_id("probe");

      return message;
    }

    /**
     * @brief Builds a reply message.
     */
    [[nodiscard]] discovery_core::DiscoveryMessage
    make_reply_message(
        const std::string &to_node_id,
        const std::string &correlation_id) const
    {
      auto payload =
          encode_announcement(
              make_local_announcement());

      auto message =
          discovery_core::DiscoveryMessage::reply(
              local_node_id(),
              std::move(payload));

      message.to_node_id = to_node_id;
      message.correlation_id = correlation_id;

      return message;
    }

    /**
     * @brief Builds an outbound envelope.
     */
    [[nodiscard]] static discovery_core::DiscoveryEnvelope
    make_outbound_envelope(
        discovery_core::DiscoveryMessage message,
        const std::string &host,
        std::uint16_t port)
    {
      return discovery_core::DiscoveryEnvelope{
          std::move(message),
          host,
          port};
    }

    /**
     * @brief Builds local announcement from current config.
     */
    [[nodiscard]] discovery_core::DiscoveryAnnouncement
    make_local_announcement() const
    {
      auto config_result = context_.config_checked();

      if (config_result.is_err())
      {
        return {};
      }

      const auto &config = *config_result.value();

      return discovery_core::DiscoveryAnnouncement{
          local_node_id(),
          config.announce_host,
          config.announce_port};
    }

    /**
     * @brief Encodes one announcement payload.
     *
     * Format:
     * - node_id string
     * - host string
     * - uint16 port
     * - int64 timestamp_millis
     */
    [[nodiscard]] static std::vector<std::uint8_t>
    encode_announcement(
        const discovery_core::DiscoveryAnnouncement &announcement)
    {
      if (!announcement.is_valid())
      {
        return {};
      }

      std::vector<std::uint8_t> buffer;

      append_string(buffer, announcement.node_id);
      append_string(buffer, announcement.host);

      store_utils::Serializer::append_u16(
          buffer,
          announcement.port);

      store_utils::Serializer::append_i64(
          buffer,
          announcement.timestamp.millis());

      return buffer;
    }

    /**
     * @brief Decodes one announcement payload.
     */
    [[nodiscard]] static std::optional<discovery_core::DiscoveryAnnouncement>
    decode_announcement(const std::vector<std::uint8_t> &buffer)
    {
      if (buffer.empty())
      {
        return std::nullopt;
      }

      return decode_announcement(
          std::span<const std::uint8_t>(buffer.data(), buffer.size()));
    }

    /**
     * @brief Decodes one announcement payload.
     */
    [[nodiscard]] static std::optional<discovery_core::DiscoveryAnnouncement>
    decode_announcement(std::span<const std::uint8_t> buffer)
    {
      std::size_t offset = 0;

      discovery_core::DiscoveryAnnouncement announcement{};

      if (!read_string(buffer, offset, announcement.node_id))
      {
        return std::nullopt;
      }

      if (!read_string(buffer, offset, announcement.host))
      {
        return std::nullopt;
      }

      if (!store_utils::Serializer::read_u16(
              buffer,
              offset,
              announcement.port))
      {
        return std::nullopt;
      }

      std::int64_t timestamp_millis = 0;

      if (!store_utils::Serializer::read_i64(
              buffer,
              offset,
              timestamp_millis))
      {
        return std::nullopt;
      }

      announcement.timestamp =
          core_time::Timestamp::from_millis(timestamp_millis);

      if (offset != buffer.size())
      {
        return std::nullopt;
      }

      if (!announcement.is_valid())
      {
        return std::nullopt;
      }

      return announcement;
    }

    /**
     * @brief Returns local node id from config.
     */
    [[nodiscard]] std::string local_node_id() const
    {
      auto config_result = context_.config_checked();

      if (config_result.is_err())
      {
        return {};
      }

      return config_result.value()->node_id;
    }

    /**
     * @brief Creates a correlation id.
     */
    [[nodiscard]] static std::string
    make_correlation_id(const std::string &prefix)
    {
      return prefix + "-" +
             std::to_string(core_time::Timestamp::now().millis());
    }

    /**
     * @brief Appends a size-prefixed string.
     */
    static void append_string(
        std::vector<std::uint8_t> &buffer,
        const std::string &value)
    {
      store_utils::Serializer::append_u32(
          buffer,
          static_cast<std::uint32_t>(value.size()));

      buffer.insert(
          buffer.end(),
          value.begin(),
          value.end());
    }

    /**
     * @brief Reads a size-prefixed string.
     */
    [[nodiscard]] static bool read_string(
        std::span<const std::uint8_t> buffer,
        std::size_t &offset,
        std::string &value)
    {
      std::uint32_t size = 0;

      if (!store_utils::Serializer::read_u32(
              buffer,
              offset,
              size))
      {
        return false;
      }

      if (!store_utils::Serializer::can_read(
              buffer,
              offset,
              size))
      {
        return false;
      }

      value.assign(
          reinterpret_cast<const char *>(buffer.data() + offset),
          size);

      offset += size;

      return true;
    }

  private:
    const discovery_core::DiscoveryContext &context_;
    discovery_backend::IDiscoveryBackend &backend_;

    discovery_client::DiscoveryClient client_;
    discovery_server::DiscoveryServer server_;
    discovery_peer::DiscoveryRegistry registry_;

    discovery_types::DiscoveryStatus status_{
        discovery_types::DiscoveryStatus::Stopped};

    core_time::Timestamp last_announce_at_{};
  };

} // namespace softadastra::discovery::engine

#endif // SOFTADASTRA_DISCOVERY_ENGINE_HPP
