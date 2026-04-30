# softadastra/discovery

> Local peer discovery layer for Softadastra.

`softadastra/discovery` is the module responsible for discovering nearby Softadastra nodes.

It allows a node to announce itself, probe the local network, receive peer announcements, maintain a registry of discovered peers, and hand usable peers to `softadastra/transport`.

The core rule is:

> *Discovery finds peers. Transport connects peers.*

## Purpose

Softadastra is designed for offline-first and local-first systems. In that model, nodes must be able to find each other without depending on a central cloud service.

The discovery module provides this first local peer discovery layer. It helps Softadastra:

- announce local node presence
- probe for nearby nodes
- receive discovery messages
- track discovered peers
- expire stale peers
- convert discovered peers into transport peer info
- integrate discovered peers with the transport engine

## What this module does

`softadastra/discovery` provides:

- discovery messages and announcements
- discovery envelopes
- datagram encoding and decoding
- peer discovery registry
- discovery backend interface
- UDP discovery backend
- discovery client and server wrappers
- discovery engine
- high-level discovery service
- public peer representation

## What this module does NOT do

- durable persistence
- store mutation
- sync queueing or conflict resolution
- TCP connection handling
- message transport after discovery
- encryption or authentication
- distributed consensus

## Design Principles

### Discovery is separate from Transport

```
Discovery  →  Who is available nearby?
Transport  →  How do I connect and send messages to this peer?
```

### Discovery is lightweight

Discovery messages carry only the minimum information needed to identify and reach a peer: `node_id`, `host`, `port`, `timestamp`.

### Discovery does not own sync

Discovery never applies sync operations. It only finds peers and gives their connection information to transport.

### Registry is observable

The discovery registry tracks peer state: `Discovered`, `Alive`, `Stale`, `Expired`.

### Layering

```
WAL        →  durability
Store      →  local state
Sync       →  operation propagation logic
Transport  →  peer message delivery
Discovery  →  peer discovery
```

Discovery sits beside transport. It feeds discovered peers into transport.

## Installation

```bash
vix add @softadastra/discovery
```

### Main header

```cpp
#include <softadastra/discovery/Discovery.hpp>
```

For full integration:

```cpp
#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>
#include <softadastra/discovery/Discovery.hpp>
```

## Module Structure

```
include/softadastra/discovery/
├── backend/
│   ├── IDiscoveryBackend.hpp
│   └── UdpDiscoveryBackend.hpp
├── client/
│   └── DiscoveryClient.hpp
├── core/
│   ├── DiscoveryAnnouncement.hpp
│   ├── DiscoveryConfig.hpp
│   ├── DiscoveryContext.hpp
│   ├── DiscoveryEnvelope.hpp
│   └── DiscoveryMessage.hpp
├── encoding/
│   ├── DiscoveryDecoder.hpp
│   └── DiscoveryEncoder.hpp
├── engine/
│   └── DiscoveryEngine.hpp
├── peer/
│   ├── DiscoveredPeer.hpp
│   └── DiscoveryRegistry.hpp
├── platform/
│   └── linux/
│       └── UdpSocket.hpp
├── server/
│   └── DiscoveryServer.hpp
├── types/
│   ├── DiscoveryMessageType.hpp
│   ├── DiscoveryPeerState.hpp
│   └── DiscoveryStatus.hpp
├── utils/
│   └── Datagram.hpp
├── Discovery.hpp
├── DiscoveryOptions.hpp
├── DiscoveryService.hpp
└── Peer.hpp
```

## Core Concepts

### `DiscoveryMessageType`

- `discovery::types::DiscoveryMessageType::Announce`
- `discovery::types::DiscoveryMessageType::Probe`
- `discovery::types::DiscoveryMessageType::Reply`

Helpers:

```cpp
discovery::types::to_string(type);
discovery::types::is_valid(type);
discovery::types::is_announcement(type);
discovery::types::is_probe_flow(type);
```

### `DiscoveryPeerState`

- `discovery::types::DiscoveryPeerState::Discovered`
- `discovery::types::DiscoveryPeerState::Alive`
- `discovery::types::DiscoveryPeerState::Stale`
- `discovery::types::DiscoveryPeerState::Expired`

Helpers:

```cpp
discovery::types::to_string(state);
discovery::types::is_valid(state);
discovery::types::is_available(state);
discovery::types::is_unavailable(state);
discovery::types::is_expired(state);
```

### `DiscoveryStatus`

- `discovery::types::DiscoveryStatus::Stopped`
- `discovery::types::DiscoveryStatus::Starting`
- `discovery::types::DiscoveryStatus::Running`
- `discovery::types::DiscoveryStatus::Stopping`
- `discovery::types::DiscoveryStatus::Failed`

Helpers:

```cpp
discovery::types::to_string(status);
discovery::types::is_valid(status);
discovery::types::is_running(status);
discovery::types::is_transitioning(status);
discovery::types::is_terminal(status);
```

## `DiscoveryConfig`

Low-level runtime configuration.

```cpp
// Local
auto config = discovery::core::DiscoveryConfig::local("node-a", 9400, 7000);

// LAN broadcast
auto config = discovery::core::DiscoveryConfig::lan("node-a", 9400, 7000);

// Customize timing
config.announce_interval = core::time::Duration::from_seconds(3);
config.peer_ttl          = core::time::Duration::from_seconds(15);

if (!config.is_valid())
{
    return 1;
}
```

Parameters: `node-a` → local node id, `9400` → UDP discovery bind port, `7000` → transport port announced to peers.

## `DiscoveryOptions`

Simpler user-facing configuration used by `DiscoveryService`.

```cpp
auto options = discovery::DiscoveryOptions::local("node-a", 9400, 7000);

auto config = options.to_config();

if (!options.is_valid())
{
    return 1;
}

// Backward-compatible alias
options.valid();
```

## `DiscoveryAnnouncement`

The payload that advertises a node. Contains: `node_id`, `host`, `port`, `timestamp`.

```cpp
discovery::core::DiscoveryAnnouncement announcement{"node-a", "127.0.0.1", 7000};

if (announcement.is_valid())
{
    announcement.touch();
}

// Local helper
auto announcement = discovery::core::DiscoveryAnnouncement::local("node-a", 7000);
```

## `DiscoveryMessage`

The logical discovery message exchanged between nodes.

```cpp
// Announcement
auto message = discovery::core::DiscoveryMessage::announce("node-a", payload);

// Probe
auto probe = discovery::core::DiscoveryMessage::probe("node-a");

// Reply
auto reply = discovery::core::DiscoveryMessage::reply("node-a", payload);

// Metadata
message.to_node_id     = "node-b";
message.correlation_id = "announce-1";

if (!message.is_valid())
{
    return 1;
}
```

## `DiscoveryEnvelope`

Wraps a discovery message with network metadata: `message`, `from_host`, `from_port`, `to_host`, `to_port`, `timestamp`, `retry_count`, `last_attempt_at`.

```cpp
discovery::core::DiscoveryEnvelope envelope{message, "127.0.0.1", 9400};

if (envelope.is_valid() && envelope.has_destination())
{
    envelope.mark_attempt();
}
```

## `Datagram`

The raw UDP payload container.

```cpp
discovery::utils::Datagram datagram{"127.0.0.1", 9400, {1, 2, 3}};

if (datagram.is_valid())
{
    auto size = datagram.payload_size();
}
```

## Encoding and Decoding

```cpp
// Encode a message
auto encoded = discovery::encoding::DiscoveryEncoder::encode_message(message);

// Wrap into a datagram
auto datagram = discovery::encoding::DiscoveryEncoder::make_datagram(
    message, "127.0.0.1", 9400);

// Decode a message
auto decoded = discovery::encoding::DiscoveryDecoder::decode_message(encoded);

if (decoded.has_value())
{
    auto type = decoded->type;
}

// Decode a datagram
auto decoded = discovery::encoding::DiscoveryDecoder::decode_datagram(datagram);
```

## `DiscoveredPeer`

Stores runtime state for one discovered peer: `announcement`, `state`, `last_seen_at`, `error_count`.

```cpp
discovery::peer::DiscoveredPeer peer{announcement};

peer.mark_alive();

if (peer.available())
{
    auto transport_peer = peer.to_peer_info();
}

// State helpers
peer.discovered();
peer.alive();
peer.stale();
peer.expired();
peer.available();

// Runtime updates
peer.mark_discovered();
peer.mark_alive();
peer.mark_stale();
peer.mark_expired();
peer.mark_error();
peer.touch();
```

## `DiscoveryRegistry`

Stores discovered peers in memory.

```cpp
discovery::peer::DiscoveryRegistry registry;

registry.upsert_announcement(announcement);

// Find a peer
auto *peer = registry.find("node-b");

if (peer != nullptr && peer->alive())
{
    // peer is alive
}

// Inspect
registry.size();
registry.available_peers();
registry.alive_peers();
registry.stale_peers();
registry.expired_peers();

// Convert to transport peers
auto transport_peers = registry.transport_peers();

// Refresh stale and expired peers
registry.refresh_states(
    core::time::Timestamp::now(),
    core::time::Duration::from_seconds(8),
    core::time::Duration::from_seconds(15));

// Remove expired peers
registry.prune_expired();
```

## Backend Interface

`IDiscoveryBackend` defines the low-level discovery delivery contract. A backend implements: `start()`, `stop()`, `is_running()`, `send()`, `poll()`.

```cpp
class MyDiscoveryBackend : public discovery::backend::IDiscoveryBackend
{
public:
    bool start() override { return true; }
    void stop() override {}
    bool is_running() const noexcept override { return true; }

    bool send(const discovery::core::DiscoveryEnvelope &envelope) override
    {
        return envelope.is_valid();
    }

    std::optional<discovery::core::DiscoveryEnvelope> poll() override
    {
        return std::nullopt;
    }
};
```

## UDP Backend

`UdpDiscoveryBackend` is the default Linux UDP backend. Currently simple and blocking.

```cpp
auto config = discovery::core::DiscoveryConfig::local("node-a", 9400, 7000);

discovery::backend::UdpDiscoveryBackend backend{config};

if (!backend.start())
{
    return 1;
}

backend.stop();
```

## `DiscoveryClient`

Thin outbound wrapper around a discovery backend.

```cpp
discovery::client::DiscoveryClient client{backend};

// Probe
client.send_probe("127.0.0.1", 9400, "node-a");

// Announce
client.send_announce(
    config.broadcast_host,
    config.broadcast_port,
    config.node_id,
    payload);

// Reply
client.send_reply("127.0.0.1", 9400, "node-a", payload);
```

## `DiscoveryServer`

Thin inbound wrapper around a discovery backend.

```cpp
discovery::server::DiscoveryServer server{backend};

server.start();

auto inbound = server.poll();

if (inbound.has_value())
{
    auto message = inbound->message;
}

server.stop();
```

## `DiscoveryContext`

Connects discovery to transport.

```cpp
discovery::core::DiscoveryContext context{discovery_config, transport_engine};

if (!context.is_valid())
{
    return 1;
}

// Checked access
auto transport = context.transport_checked();

if (transport.is_ok())
{
    auto *engine = transport.value();
}
```

## `DiscoveryEngine`

The high-level discovery facade. Coordinates: backend lifecycle, announce/probe messages, inbound polling, peer registry updates, and transport integration.

```cpp
auto discovery_config = discovery::core::DiscoveryConfig::local("node-a", 9400, 7000);

discovery::core::DiscoveryContext discovery_context{
    discovery_config,
    transport_engine};

discovery::backend::UdpDiscoveryBackend discovery_backend{discovery_config};

discovery::engine::DiscoveryEngine discovery_engine{
    discovery_context,
    discovery_backend};

if (!discovery_engine.start())
{
    return 1;
}

discovery_engine.announce_now();
discovery_engine.probe_now();
discovery_engine.poll_once();

auto processed = discovery_engine.poll_many(16);

auto peers = discovery_engine.available_transport_peers();

discovery_engine.stop();
```

## `DiscoveryService`

The simple user-facing wrapper. Owns: `DiscoveryOptions`, `DiscoveryConfig`, `DiscoveryContext`, `UdpDiscoveryBackend`, `DiscoveryEngine`. Does **not** own `TransportEngine`.

```cpp
auto options = discovery::DiscoveryOptions::local("node-a", 9400, 7000);

discovery::DiscoveryService service{options, transport_engine};

service.on_peer_found([](const discovery::Peer &peer) {
    // peer discovered
});

service.start();
service.announce_now();
service.probe_now();
service.poll_many(16);
service.stop();

// Get public peers
auto peers = service.peers();

// Check running state
if (service.is_running()) {}

// Backward-compatible alias
service.running();
```

## Public Peer

`discovery::Peer` is the simple public peer representation.

```cpp
discovery::Peer peer{"node-b", "127.0.0.1", 7001};

if (peer.is_valid())
{
    auto transport_peer = peer.to_transport_peer();
}

// Convert from internal peer
auto public_peer = discovery::Peer::from_discovered_peer(discovered_peer);
```

## Full Integration Example

```cpp
#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>
#include <softadastra/discovery/Discovery.hpp>

using namespace softadastra;

int main()
{
    const std::string wal_path = "node-a.wal";
    std::filesystem::remove(wal_path);

    store::engine::StoreEngine store{
        store::core::StoreConfig::durable(wal_path)};

    auto sync_config = sync::core::SyncConfig::durable("node-a");

    sync::core::SyncContext  sync_context{store, sync_config};
    sync::engine::SyncEngine sync_engine{sync_context};

    auto transport_config = transport::core::TransportConfig::local(7000);

    transport::core::TransportContext transport_context{
        transport_config,
        sync_engine};

    transport::backend::TcpTransportBackend transport_backend{transport_config};

    transport::engine::TransportEngine transport_engine{
        transport_context,
        transport_backend};

    if (!transport_engine.start())
    {
        return 1;
    }

    auto discovery_options =
        discovery::DiscoveryOptions::local("node-a", 9400, 7000);

    discovery::DiscoveryService discovery_service{
        discovery_options,
        transport_engine};

    discovery_service.on_peer_found([](const discovery::Peer &peer) {
        std::cout << "peer found: "
                  << peer.node_id << " "
                  << peer.host   << ":"
                  << peer.port   << "\n";
    });

    if (!discovery_service.start())
    {
        transport_engine.stop();
        return 1;
    }

    discovery_service.announce_now();
    discovery_service.probe_now();
    discovery_service.poll_many(8);

    discovery_service.stop();
    transport_engine.stop();

    std::filesystem::remove(wal_path);

    return 0;
}
```

## Discovery Flow

```
1.  Node starts TransportEngine
2.  Node starts DiscoveryEngine or DiscoveryService
3.  Node sends Announce over UDP
4.  Other nodes receive the announcement
5.  DiscoveryRegistry stores or refreshes the peer
6.  DiscoveredPeer becomes Alive
7.  DiscoveryEngine converts it to transport::core::PeerInfo
8.  TransportEngine connects to the peer
9.  Sync can now send messages through Transport
```

## Probe Flow

```
1.  Node sends Probe
2.  Nearby nodes receive Probe
3.  Each node replies with Reply
4.  Reply contains an encoded DiscoveryAnnouncement
5.  Receiver stores the peer
6.  Receiver integrates peer with Transport
```

## Peer Expiration

Peers are refreshed on every valid announcement or reply.

| Condition | State |
|-----------|-------|
| Recently seen | `Alive` |
| Not seen for half TTL | `Stale` |
| Not seen for full TTL | `Expired` |

```cpp
registry.prune_expired();
```

## Examples

| Example | Description |
|---------|-------------|
| `discovery_minimal.cpp` | Minimal setup |
| `discovery_announcer.cpp` | Announce only |
| `discovery_listener.cpp` | Listen only |
| `discovery_roundtrip_demo.cpp` | Full roundtrip |
| `discovery_codec.cpp` | Encoding and decoding |
| `discovery_registry.cpp` | Registry management |
| `discovery_service.cpp` | Service usage |
| `discovery_engine_setup.cpp` | Engine setup |

```bash
vix build
```

## Production Notes

The current UDP backend is intentionally simple. Recommended next steps:

- async UDP backend
- multicast backend
- configurable network interfaces
- secure peer identity and signed announcements
- encrypted discovery payloads
- rate limiting
- structured metrics and tracing
- persistent peer cache
- LAN and edge discovery modes
- P2P discovery integration

## Design Rules

- Discovery finds peers
- Transport connects peers
- Sync owns operation meaning
- Store owns state
- WAL owns durability
- Discovery must not mutate store
- Discovery must not apply sync operations
- Discovery must not resolve conflicts
- Discovery must keep peer state observable
- Backends must not contain registry logic
- Backends must not contain transport integration logic

## Dependencies

**Internal:**
- `softadastra/core`
- `softadastra/store`
- `softadastra/transport`

**External:**
- C++20 standard library
- Linux UDP sockets (current backend)

## Roadmap

- [ ] Public `Discovery.hpp` aggregator
- [ ] Stable high-level `DiscoveryService`
- [ ] UDP broadcast backend
- [ ] Local development discovery mode
- [ ] LAN discovery mode
- [ ] Multicast discovery backend
- [ ] Persistent peer cache
- [ ] Discovery metrics and tracing
- [ ] Signed and encrypted announcements
- [ ] Peer trust policy
- [ ] P2P and edge node discovery
- [ ] NAT-aware discovery helpers

## Summary

`softadastra/discovery` provides:

- discovery messages and announcements
- discovery envelopes and datagrams
- encoding and decoding
- peer registry
- UDP backend
- client/server wrappers
- discovery engine
- high-level discovery service

> Its job is simple: find peers and hand them to transport.
