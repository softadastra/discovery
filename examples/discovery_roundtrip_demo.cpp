/*
 * discovery_roundtrip_demo.cpp
 *
 * Roundtrip discovery demo in one process:
 * - node A starts transport + discovery
 * - node B starts transport + discovery
 * - both nodes probe / announce on the same discovery port
 * - both nodes poll until peers are discovered
 * - final discovered state is printed
 */

#include <chrono>
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
  struct DemoNode
  {
    std::string node_id;
    std::uint16_t transport_port;
    std::uint16_t discovery_port;

    store::engine::StoreEngine store;
    sync::core::SyncConfig sync_config;
    sync::core::SyncContext sync_context;
    sync::engine::SyncEngine sync;

    transport::core::TransportConfig transport_config;
    transport::core::TransportContext transport_context;
    transport::backend::TcpTransportBackend transport_backend;
    transport::engine::TransportEngine transport_engine;

    discovery::core::DiscoveryConfig discovery_config;
    discovery::core::DiscoveryContext discovery_context;
    discovery::backend::UdpDiscoveryBackend discovery_backend;
    discovery::engine::DiscoveryEngine discovery_engine;

    DemoNode(const std::string &id,
             std::uint16_t tport,
             std::uint16_t dport)
        : node_id(id),
          transport_port(tport),
          discovery_port(dport),
          store(make_store_config()),
          sync_config(make_sync_config(id)),
          sync_context(make_sync_context(store, sync_config)),
          sync(sync_context),
          transport_config(make_transport_config(tport)),
          transport_context(make_transport_context(sync, transport_config)),
          transport_backend(transport_config),
          transport_engine(transport_context, transport_backend),
          discovery_config(make_discovery_config(id, tport, dport)),
          discovery_context(make_discovery_context(transport_engine, discovery_config)),
          discovery_backend(discovery_config),
          discovery_engine(discovery_context, discovery_backend)
    {
    }

    static store::core::StoreConfig make_store_config()
    {
      store::core::StoreConfig config;
      config.enable_wal = false;
      return config;
    }

    static sync::core::SyncConfig make_sync_config(const std::string &node_id)
    {
      sync::core::SyncConfig config;
      config.node_id = node_id;
      config.auto_queue = true;
      config.require_ack = true;
      return config;
    }

    static sync::core::SyncContext make_sync_context(store::engine::StoreEngine &store,
                                                     sync::core::SyncConfig &config)
    {
      sync::core::SyncContext context;
      context.store = &store;
      context.config = &config;
      return context;
    }

    static transport::core::TransportConfig make_transport_config(std::uint16_t port)
    {
      transport::core::TransportConfig config;
      config.bind_host = "0.0.0.0";
      config.bind_port = port;
      return config;
    }

    static transport::core::TransportContext make_transport_context(
        sync::engine::SyncEngine &sync,
        transport::core::TransportConfig &config)
    {
      transport::core::TransportContext context;
      context.config = &config;
      context.sync = &sync;
      return context;
    }

    static discovery::core::DiscoveryConfig make_discovery_config(
        const std::string &node_id,
        std::uint16_t transport_port,
        std::uint16_t discovery_port)
    {
      discovery::core::DiscoveryConfig config;
      config.bind_host = "0.0.0.0";
      config.bind_port = discovery_port;
      config.broadcast_host = "255.255.255.255";
      config.broadcast_port = discovery_port;
      config.node_id = node_id;
      config.announce_host = "127.0.0.1";
      config.announce_port = transport_port;
      config.announce_interval_ms = 2000;
      config.peer_ttl_ms = 10000;
      config.max_datagram_size = 64 * 1024;
      config.enable_broadcast = true;
      return config;
    }

    static discovery::core::DiscoveryContext make_discovery_context(
        transport::engine::TransportEngine &transport_engine,
        discovery::core::DiscoveryConfig &config)
    {
      discovery::core::DiscoveryContext context;
      context.config = &config;
      context.transport = &transport_engine;
      return context;
    }

    bool start()
    {
      if (!transport_engine.start())
      {
        return false;
      }

      if (!discovery_engine.start())
      {
        transport_engine.stop();
        return false;
      }

      return true;
    }

    void stop()
    {
      discovery_engine.stop();
      transport_engine.stop();
    }

    void tick()
    {
      discovery_engine.poll_many(16);
      transport_engine.poll_many(16);
    }

    bool announce()
    {
      return discovery_engine.announce_now();
    }

    bool probe()
    {
      return discovery_engine.probe_now();
    }
  };

  void print_node_peers(const DemoNode &node)
  {
    const auto peers = node.discovery_engine.peers().all();

    std::cout << "[" << node.node_id << "] discovered peers:\n";

    if (peers.empty())
    {
      std::cout << "  - none\n";
      return;
    }

    for (const auto &peer : peers)
    {
      std::cout
          << "  - node_id=" << peer.announcement.node_id
          << ", host=" << peer.announcement.host
          << ", port=" << peer.announcement.port
          << ", last_seen_at=" << peer.last_seen_at
          << '\n';
    }
  }

  bool has_peer(const DemoNode &node, const std::string &peer_id)
  {
    return node.discovery_engine.peers().contains(peer_id);
  }
}

int main()
{
  try
  {
    using namespace std::chrono_literals;

    DemoNode node_a("node-a", 9301, 9400);
    DemoNode node_b("node-b", 9302, 9400);

    if (!node_a.start())
    {
      std::cerr << "[demo] failed to start node-a\n";
      return 1;
    }

    if (!node_b.start())
    {
      std::cerr << "[demo] failed to start node-b\n";
      node_a.stop();
      return 1;
    }

    std::cout << "[demo] both nodes started\n";

    node_a.probe();
    node_b.probe();

    bool converged = false;

    for (int i = 0; i < 50; ++i)
    {
      node_a.announce();
      node_b.announce();

      node_a.tick();
      node_b.tick();

      if (has_peer(node_a, "node-b") && has_peer(node_b, "node-a"))
      {
        converged = true;
        break;
      }

      std::this_thread::sleep_for(200ms);
    }

    print_node_peers(node_a);
    print_node_peers(node_b);

    node_a.stop();
    node_b.stop();

    if (!converged)
    {
      std::cerr << "[demo] discovery roundtrip did not converge\n";
      return 1;
    }

    std::cout << "[demo] discovery roundtrip completed successfully\n";
    return 0;
  }
  catch (const std::exception &ex)
  {
    std::cerr << "[demo] fatal exception: " << ex.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "[demo] fatal unknown exception\n";
    return 1;
  }
}
