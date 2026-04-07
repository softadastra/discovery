/*
 * DiscoveryService.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_SERVICE_HPP
#define SOFTADASTRA_DISCOVERY_SERVICE_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <softadastra/discovery/DiscoveryOptions.hpp>
#include <softadastra/discovery/Peer.hpp>
#include <softadastra/discovery/backend/UdpDiscoveryBackend.hpp>
#include <softadastra/discovery/core/DiscoveryContext.hpp>
#include <softadastra/discovery/engine/DiscoveryEngine.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

namespace softadastra::discovery
{
  namespace transport_engine = softadastra::transport::engine;
  namespace discovery_backend = softadastra::discovery::backend;
  namespace discovery_core = softadastra::discovery::core;
  namespace discovery_engine = softadastra::discovery::engine;

  class DiscoveryService
  {
  public:
    using PeerFoundHandler = std::function<void(const Peer &)>;

    DiscoveryService(const DiscoveryOptions &options,
                     transport_engine::TransportEngine &transport);

    bool start();
    void stop();
    bool running() const noexcept;

    bool announce_now();
    bool probe_now();

    bool poll();
    std::size_t poll_many(std::size_t max_messages);

    std::vector<Peer> peers() const;

    void onPeerFound(PeerFoundHandler handler);

  private:
    Peer to_public_peer(
        const softadastra::discovery::peer::DiscoveredPeer &peer) const;

  private:
    DiscoveryOptions options_;
    discovery_core::DiscoveryConfig config_;
    discovery_core::DiscoveryContext context_;
    discovery_backend::UdpDiscoveryBackend backend_;
    discovery_engine::DiscoveryEngine engine_;

    PeerFoundHandler on_peer_found_;
    std::size_t known_peer_count_{0};
  };

} // namespace softadastra::discovery

#endif
