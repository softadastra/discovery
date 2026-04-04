/*
 * discovery_listener.cpp
 *
 * Simple discovery listener:
 * - starts discovery
 * - listens for incoming announcements / replies
 * - prints discovered peers
 */

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

#include <softadastra/discovery/backend/UdpDiscoveryBackend.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/discovery/core/DiscoveryContext.hpp>
#include <softadastra/discovery/engine/DiscoveryEngine.hpp>

#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>

#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>

#include <softadastra/transport/backend/TcpTransportBackend.hpp>
#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/transport/core/TransportContext.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

using namespace softadastra;

namespace
{
  std::atomic<bool> g_running{true};

  constexpr const char *kNodeId = "discovery-listener";
  constexpr std::uint16_t kTransportPort = 9101;
  constexpr std::uint16_t kDiscoveryPort = 9200;

  void handle_signal(int)
  {
    g_running = false;
  }

  void install_signal_handlers()
  {
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);
  }

  void log_info(const std::string &message)
  {
    std::cout << "[listener] " << message << '\n';
  }

  store::core::StoreConfig build_store_config()
  {
    store::core::StoreConfig config;
    config.enable_wal = false;
    return config;
  }

  sync::core::SyncConfig build_sync_config()
  {
    sync::core::SyncConfig config;
    config.node_id = kNodeId;
    config.auto_queue = true;
    config.require_ack = true;
    return config;
  }

  transport::core::TransportConfig build_transport_config()
  {
    transport::core::TransportConfig config;
    config.bind_host = "0.0.0.0";
    config.bind_port = kTransportPort;
    return config;
  }

  discovery::core::DiscoveryConfig build_discovery_config()
  {
    discovery::core::DiscoveryConfig config;
    config.bind_host = "0.0.0.0";
    config.bind_port = kDiscoveryPort;
    config.broadcast_host = "255.255.255.255";
    config.broadcast_port = kDiscoveryPort;
    config.node_id = kNodeId;
    config.announce_host = "127.0.0.1";
    config.announce_port = kTransportPort;
    config.announce_interval_ms = 3000;
    config.peer_ttl_ms = 15000;
    config.max_datagram_size = 64 * 1024;
    config.enable_broadcast = true;
    return config;
  }

  void print_peers(const discovery::engine::DiscoveryEngine &engine)
  {
    const auto peers = engine.peers().all();

    if (peers.empty())
    {
      log_info("discovered peers: none");
      return;
    }

    log_info("discovered peers:");

    for (const auto &peer : peers)
    {
      std::cout
          << "  - node_id=" << peer.announcement.node_id
          << ", host=" << peer.announcement.host
          << ", port=" << peer.announcement.port
          << ", last_seen_at=" << peer.last_seen_at
          << ", error_count=" << peer.error_count
          << '\n';
    }
  }
}

int main()
{
  try
  {
    using namespace std::chrono_literals;

    install_signal_handlers();

    log_info("initializing store...");
    store::engine::StoreEngine store(build_store_config());

    log_info("initializing sync...");
    sync::core::SyncConfig sync_config = build_sync_config();
    sync::core::SyncContext sync_context;
    sync_context.store = &store;
    sync_context.config = &sync_config;
    sync::engine::SyncEngine sync(sync_context);

    log_info("initializing transport...");
    transport::core::TransportConfig transport_config = build_transport_config();
    transport::core::TransportContext transport_context;
    transport_context.config = &transport_config;
    transport_context.sync = &sync;

    transport::backend::TcpTransportBackend transport_backend(transport_config);
    transport::engine::TransportEngine transport_engine(transport_context, transport_backend);

    if (!transport_engine.start())
    {
      std::cerr << "[listener] failed to start transport engine\n";
      return 1;
    }

    log_info("initializing discovery...");
    discovery::core::DiscoveryConfig discovery_config = build_discovery_config();
    discovery::core::DiscoveryContext discovery_context;
    discovery_context.config = &discovery_config;
    discovery_context.transport = &transport_engine;

    discovery::backend::UdpDiscoveryBackend discovery_backend(discovery_config);
    discovery::engine::DiscoveryEngine discovery_engine(discovery_context, discovery_backend);

    if (!discovery_engine.start())
    {
      std::cerr << "[listener] failed to start discovery engine\n";
      return 1;
    }

    log_info("listener started");
    log_info("waiting for discovery messages...");

    std::size_t tick = 0;

    while (g_running)
    {
      discovery_engine.poll_many(16);
      transport_engine.poll_many(16);

      if (tick % 20 == 0)
      {
        print_peers(discovery_engine);
      }

      ++tick;
      std::this_thread::sleep_for(200ms);
    }

    discovery_engine.stop();
    transport_engine.stop();

    log_info("listener stopped cleanly");
    return 0;
  }
  catch (const std::exception &ex)
  {
    std::cerr << "[listener] fatal exception: " << ex.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "[listener] fatal unknown exception\n";
    return 1;
  }
}
