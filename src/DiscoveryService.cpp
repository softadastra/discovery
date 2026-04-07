/*
 * DiscoveryService.cpp
 */

#include <softadastra/discovery/DiscoveryService.hpp>

#include <unordered_set>

namespace softadastra::discovery
{
  namespace discovery_peer = softadastra::discovery::peer;
  namespace discovery_types = softadastra::discovery::types;

  namespace
  {
    discovery_core::DiscoveryConfig make_config(const DiscoveryOptions &options)
    {
      discovery_core::DiscoveryConfig config;
      config.bind_host = options.bind_host;
      config.bind_port = options.bind_port;
      config.broadcast_host = options.broadcast_host;
      config.broadcast_port = options.broadcast_port;
      config.node_id = options.node_id;
      config.announce_host = options.announce_host;
      config.announce_port = options.announce_port;
      config.announce_interval_ms = options.announce_interval_ms;
      config.peer_ttl_ms = options.peer_ttl_ms;
      config.enable_broadcast = options.enable_broadcast;
      return config;
    }
  }

  DiscoveryService::DiscoveryService(const DiscoveryOptions &options,
                                     transport_engine::TransportEngine &transport)
      : options_(options),
        config_(make_config(options)),
        context_{},
        backend_(config_),
        engine_([&]() -> discovery_core::DiscoveryContext &
                {
          context_.config = &config_;
          context_.transport = &transport;
          return context_; }(),
                backend_)
  {
  }

  bool DiscoveryService::start()
  {
    if (!options_.valid())
    {
      return false;
    }

    return engine_.start();
  }

  softadastra::discovery::engine::DiscoveryEngine &DiscoveryService::engine() noexcept
  {
    return engine_;
  }

  const softadastra::discovery::engine::DiscoveryEngine &DiscoveryService::engine() const noexcept
  {
    return engine_;
  }

  void DiscoveryService::stop()
  {
    engine_.stop();
    known_peer_count_ = 0;
  }

  bool DiscoveryService::running() const noexcept
  {
    return engine_.running();
  }

  bool DiscoveryService::announce_now()
  {
    return engine_.announce_now();
  }

  bool DiscoveryService::probe_now()
  {
    return engine_.probe_now();
  }

  bool DiscoveryService::poll()
  {
    const auto before = engine_.peers().all();
    const bool processed = engine_.poll_once();
    const auto after = engine_.peers().all();

    if (processed && on_peer_found_ && after.size() > before.size())
    {
      for (const auto &peer : after)
      {
        if (peer.alive())
        {
          on_peer_found_(to_public_peer(peer));
        }
      }
    }

    known_peer_count_ = after.size();
    return processed;
  }

  std::size_t DiscoveryService::poll_many(std::size_t max_messages)
  {
    std::size_t processed = 0;

    for (std::size_t i = 0; i < max_messages; ++i)
    {
      if (!poll())
      {
        break;
      }

      ++processed;
    }

    return processed;
  }

  std::vector<Peer> DiscoveryService::peers() const
  {
    std::vector<Peer> result;

    for (const auto &peer : engine_.peers().all())
    {
      if (peer.valid())
      {
        result.push_back(to_public_peer(peer));
      }
    }

    return result;
  }

  void DiscoveryService::onPeerFound(PeerFoundHandler handler)
  {
    on_peer_found_ = std::move(handler);
  }

  Peer DiscoveryService::to_public_peer(
      const discovery_peer::DiscoveredPeer &peer) const
  {
    Peer result;
    result.node_id = peer.announcement.node_id;
    result.host = peer.announcement.host;
    result.port = peer.announcement.port;
    result.last_seen_at = peer.last_seen_at;
    return result;
  }

} // namespace softadastra::discovery
