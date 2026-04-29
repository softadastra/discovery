/**
 *
 *  @file DiscoveryService.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_SERVICE_HPP
#define SOFTADASTRA_DISCOVERY_SERVICE_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <softadastra/discovery/DiscoveryOptions.hpp>
#include <softadastra/discovery/Peer.hpp>
#include <softadastra/discovery/backend/UdpDiscoveryBackend.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/discovery/core/DiscoveryContext.hpp>
#include <softadastra/discovery/engine/DiscoveryEngine.hpp>
#include <softadastra/discovery/peer/DiscoveredPeer.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

namespace softadastra::discovery
{
  namespace transport_engine = softadastra::transport::engine;
  namespace discovery_backend = softadastra::discovery::backend;
  namespace discovery_core = softadastra::discovery::core;
  namespace discovery_engine = softadastra::discovery::engine;
  namespace discovery_peer = softadastra::discovery::peer;

  /**
   * @brief High-level discovery service.
   *
   * DiscoveryService is the simple user-facing wrapper around the lower-level
   * discovery engine.
   *
   * It owns:
   * - DiscoveryOptions
   * - DiscoveryConfig
   * - DiscoveryContext
   * - UdpDiscoveryBackend
   * - DiscoveryEngine
   *
   * It does not own the TransportEngine.
   * The transport engine must outlive this service.
   *
   * Typical flow:
   *
   * @code
   * DiscoveryService discovery{options, transport};
   *
   * discovery.onPeerFound([](const discovery::Peer &peer)
   * {
   *   // peer discovered
   * });
   *
   * discovery.start();
   * discovery.announce_now();
   * discovery.poll_many(16);
   * discovery.stop();
   * @endcode
   */
  class DiscoveryService
  {
  public:
    /**
     * @brief Handler called when new peers are observed.
     */
    using PeerFoundHandler = std::function<void(const Peer &)>;

    /**
     * @brief Creates a discovery service.
     *
     * @param options Public discovery options.
     * @param transport Transport engine used for peer integration.
     */
    DiscoveryService(
        DiscoveryOptions options,
        transport_engine::TransportEngine &transport)
        : options_(std::move(options)),
          config_(options_.to_config()),
          context_(config_, transport),
          backend_(config_),
          engine_(context_, backend_)
    {
    }

    /**
     * @brief Returns read-only access to the underlying engine.
     *
     * @return DiscoveryEngine const reference.
     */
    [[nodiscard]] const discovery_engine::DiscoveryEngine &
    engine() const noexcept
    {
      return engine_;
    }

    /**
     * @brief Returns mutable access to the underlying engine.
     *
     * @return DiscoveryEngine reference.
     */
    [[nodiscard]] discovery_engine::DiscoveryEngine &
    engine() noexcept
    {
      return engine_;
    }

    /**
     * @brief Starts discovery.
     *
     * @return true on success.
     */
    bool start()
    {
      if (!options_.is_valid())
      {
        return false;
      }

      known_peer_count_ = engine_.peers().size();

      return engine_.start();
    }

    /**
     * @brief Stops discovery.
     */
    void stop()
    {
      engine_.stop();
      known_peer_count_ = 0;
    }

    /**
     * @brief Returns whether discovery is running.
     *
     * @return true when running.
     */
    [[nodiscard]] bool is_running() const noexcept
    {
      return engine_.is_running();
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
     * @brief Sends one announcement immediately.
     *
     * @return true if sent.
     */
    bool announce_now()
    {
      return engine_.announce_now();
    }

    /**
     * @brief Sends one probe immediately.
     *
     * @return true if sent.
     */
    bool probe_now()
    {
      return engine_.probe_now();
    }

    /**
     * @brief Polls one discovery message.
     *
     * @return true if one message was processed.
     */
    bool poll()
    {
      const bool processed = engine_.poll_once();

      notify_new_peers();

      return processed;
    }

    /**
     * @brief Polls up to max_messages discovery messages.
     *
     * @param max_messages Maximum number of messages to process.
     * @return Number of successfully processed messages.
     */
    std::size_t poll_many(std::size_t max_messages)
    {
      const auto processed =
          engine_.poll_many(max_messages);

      notify_new_peers();

      return processed;
    }

    /**
     * @brief Returns all currently known public peers.
     *
     * @return Public peer list.
     */
    [[nodiscard]] std::vector<Peer> peers() const
    {
      std::vector<Peer> result;

      const auto discovered =
          engine_.peers().available_peers();

      result.reserve(discovered.size());

      for (const auto &peer : discovered)
      {
        auto public_peer = to_public_peer(peer);

        if (public_peer.is_valid())
        {
          result.push_back(std::move(public_peer));
        }
      }

      return result;
    }

    /**
     * @brief Installs a peer-found callback.
     *
     * @param handler Callback called when new peers are observed.
     */
    void onPeerFound(PeerFoundHandler handler)
    {
      on_peer_found_ = std::move(handler);
    }

    /**
     * @brief Installs a peer-found callback.
     *
     * Snake_case alias for newer code.
     *
     * @param handler Callback called when new peers are observed.
     */
    void on_peer_found(PeerFoundHandler handler)
    {
      onPeerFound(std::move(handler));
    }

    /**
     * @brief Returns the public options.
     *
     * @return Discovery options.
     */
    [[nodiscard]] const DiscoveryOptions &
    options() const noexcept
    {
      return options_;
    }

    /**
     * @brief Returns the core discovery config.
     *
     * @return Discovery config.
     */
    [[nodiscard]] const discovery_core::DiscoveryConfig &
    config() const noexcept
    {
      return config_;
    }

  private:
    /**
     * @brief Converts an internal discovered peer to public Peer.
     *
     * @param peer Internal discovered peer.
     * @return Public peer.
     */
    [[nodiscard]] Peer to_public_peer(
        const discovery_peer::DiscoveredPeer &peer) const
    {
      return Peer::from_discovered_peer(peer);
    }

    /**
     * @brief Notifies the callback for newly observed peers.
     */
    void notify_new_peers()
    {
      if (!on_peer_found_)
      {
        known_peer_count_ = engine_.peers().available_peers().size();
        return;
      }

      const auto discovered =
          engine_.peers().available_peers();

      if (discovered.size() <= known_peer_count_)
      {
        known_peer_count_ = discovered.size();
        return;
      }

      for (std::size_t i = known_peer_count_;
           i < discovered.size();
           ++i)
      {
        auto public_peer =
            to_public_peer(discovered[i]);

        if (public_peer.is_valid())
        {
          on_peer_found_(public_peer);
        }
      }

      known_peer_count_ = discovered.size();
    }

  private:
    DiscoveryOptions options_{};
    discovery_core::DiscoveryConfig config_{};
    discovery_core::DiscoveryContext context_{};
    discovery_backend::UdpDiscoveryBackend backend_;
    discovery_engine::DiscoveryEngine engine_;

    PeerFoundHandler on_peer_found_{};
    std::size_t known_peer_count_{0};
  };

} // namespace softadastra::discovery

#endif // SOFTADASTRA_DISCOVERY_SERVICE_HPP
