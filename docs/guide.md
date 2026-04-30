# Discovery Guide

The Softadastra Discovery module finds nearby nodes and hands them to the transport layer.

The core rule is:

> *Discovery finds peers. Transport connects peers.*

## Why Discovery exists

Softadastra is built for local-first and offline-first systems. In that model, nodes must be able to discover each other without depending on a central cloud server.

Discovery provides that first local peer discovery layer. It allows a node to:

- announce itself
- probe the local network
- receive peer announcements
- track discovered peers
- expire stale peers
- convert discovered peers into transport peers
- hand peers to `softadastra/transport`

## Layering

```
WAL        →  durable operation log
Store      →  local state
Sync       →  operation propagation
Transport  →  peer message delivery
Discovery  →  peer discovery
```

> Discovery does not apply operations.
> Discovery does not connect TCP peers directly.
> Discovery only finds peers and gives their information to transport.

## What Discovery does

`softadastra/discovery` provides:

- discovery message types and peer states
- discovery announcements, messages, and envelopes
- UDP datagrams
- message encoding and decoding
- discovered peer registry
- UDP discovery backend
- discovery client and server
- discovery engine
- high-level discovery service
- public peer object

## What Discovery does NOT do

- WAL persistence
- store mutation
- sync conflict resolution or queueing
- TCP transport
- sync message delivery after peer discovery
- encryption or authentication
- consensus

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

## Main concepts

- `DiscoveryMessageType`
- `DiscoveryPeerState`
- `DiscoveryStatus`
- `DiscoveryConfig`
- `DiscoveryOptions`
- `DiscoveryAnnouncement`
- `DiscoveryMessage`
- `DiscoveryEnvelope`
- `Datagram`
- `DiscoveryEncoder` / `DiscoveryDecoder`
- `DiscoveredPeer`
- `DiscoveryRegistry`
- `IDiscoveryBackend`
- `UdpDiscoveryBackend`
- `DiscoveryClient`
- `DiscoveryServer`
- `DiscoveryContext`
- `DiscoveryEngine`
- `DiscoveryService`
- `Peer`

## Message types

Discovery supports three message types:

| Type | Meaning |
|------|---------|
| `Announce` | "I exist. Here is my node id, host, and port." |
| `Probe` | "Who is available nearby?" |
| `Reply` | Answer to a probe with local peer information. |

Helpers:

```cpp
discovery::types::to_string(type);
discovery::types::is_valid(type);
discovery::types::is_announcement(type);
discovery::types::is_probe_flow(type);
```

## Peer states

| State | Meaning |
|-------|---------|
| `Discovered` | Seen at least once |
| `Alive` | Seen recently, usable |
| `Stale` | Not seen recently, not yet expired |
| `Expired` | Exceeded TTL, should be ignored or removed |

Helpers:

```cpp
discovery::types::to_string(state);
discovery::types::is_valid(state);
discovery::types::is_available(state);
discovery::types::is_unavailable(state);
discovery::types::is_expired(state);
```

## Discovery status

- `Stopped`, `Starting`, `Running`, `Stopping`, `Failed`

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
// Local development
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

User-facing configuration used by `DiscoveryService`.

```cpp
auto options = discovery::DiscoveryOptions::local("node-a", 9400, 7000);
auto options = discovery::DiscoveryOptions::lan("node-a", 9400, 7000);

auto config = options.to_config();

if (!options.is_valid()) { return 1; }

// Backward-compatible alias
options.valid();
```

## `DiscoveryAnnouncement`

The payload that advertises a node. Contains: `node_id`, `host`, `port`, `timestamp`.

```cpp
discovery::core::DiscoveryAnnouncement announcement{"node-a", "127.0.0.1", 7000};

// Local helper
auto announcement = discovery::core::DiscoveryAnnouncement::local("node-a", 7000);

if (!announcement.is_valid()) { return 1; }

announcement.touch();
```

## `DiscoveryMessage`

The logical discovery message exchanged between nodes.

```cpp
auto message = discovery::core::DiscoveryMessage::announce("node-a", payload);
auto probe   = discovery::core::DiscoveryMessage::probe("node-a");
auto reply   = discovery::core::DiscoveryMessage::reply("node-a", payload);

message.to_node_id     = "node-b";
message.correlation_id = "announce-1";

if (!message.is_valid()) { return 1; }
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

Raw UDP payload container used by the backend.

```cpp
discovery::utils::Datagram datagram{"127.0.0.1", 9400, {1, 2, 3}};

if (datagram.is_valid())
{
    auto size = datagram.payload_size();
}
```

## Encoding

```cpp
// Encode a message
auto encoded = discovery::encoding::DiscoveryEncoder::encode_message(message);

// Wrap into a datagram
auto datagram = discovery::encoding::DiscoveryEncoder::make_datagram(
    message, "127.0.0.1", 9400);
```

## Decoding

```cpp
// Decode from bytes
auto decoded = discovery::encoding::DiscoveryDecoder::decode_message(encoded);

// Decode from datagram
auto decoded = discovery::encoding::DiscoveryDecoder::decode_datagram(datagram);
```

## `DiscoveredPeer`

Internal runtime representation of one discovered peer.

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

peer.update(new_announcement);
auto transport_peer = peer.to_peer_info();
```

## `DiscoveryRegistry`

Stores discovered peers in memory.

```cpp
discovery::peer::DiscoveryRegistry registry;

registry.upsert_announcement(announcement);

// Find without copying
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

// Refresh states
registry.refresh_states(
    core::time::Timestamp::now(),
    core::time::Duration::from_seconds(8),
    core::time::Duration::from_seconds(15));

// Prune expired
registry.prune_expired();
```

## Backend interface

`IDiscoveryBackend` defines the low-level discovery backend contract. A backend must implement: `start()`, `stop()`, `is_running()`, `send()`, `poll()`.

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

## UDP backend

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

client.send_probe("127.0.0.1", 9400, "node-a");

client.send_announce(
    config.broadcast_host,
    config.broadcast_port,
    config.node_id,
    payload);

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
auto peers     = discovery_engine.available_transport_peers();

discovery_engine.stop();
```

## `DiscoveryService`

The simple user-facing API. Owns: `DiscoveryOptions`, `DiscoveryConfig`, `DiscoveryContext`, `UdpDiscoveryBackend`, `DiscoveryEngine`. Does **not** own `TransportEngine` — the transport engine must outlive the discovery service.

```cpp
auto options = discovery::DiscoveryOptions::local("node-a", 9400, 7000);

discovery::DiscoveryService service{options, transport_engine};

service.on_peer_found([](const discovery::Peer &peer) {
    // peer discovered
});

if (!service.start())
{
    return 1;
}

service.announce_now();
service.probe_now();
service.poll();
service.poll_many(16);

auto peers = service.peers();

service.stop();
```

## Public Peer

`discovery::Peer` is the simple public representation of a discovered peer.

```cpp
discovery::Peer peer{"node-b", "127.0.0.1", 7001};

if (!peer.is_valid())
{
    return 1;
}

auto transport_peer = peer.to_transport_peer();
auto public_peer    = discovery::Peer::from_discovered_peer(discovered_peer);
```

## Full integration setup

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

## Discovery flow

```
1.  TransportEngine starts
2.  DiscoveryEngine or DiscoveryService starts
3.  Node sends Announce over UDP
4.  Other nodes receive the announcement
5.  DiscoveryRegistry stores or refreshes the peer
6.  DiscoveredPeer becomes Alive
7.  DiscoveryEngine converts it to transport::core::PeerInfo
8.  TransportEngine connects to the peer
9.  Sync can send messages through Transport
```

## Probe flow

```
1.  Node sends Probe
2.  Nearby nodes receive Probe
3.  Each node replies with Reply
4.  Reply contains an encoded DiscoveryAnnouncement
5.  Receiver stores the peer
6.  Receiver integrates peer with Transport
```

## Expiration flow

| Condition | State |
|-----------|-------|
| Recently seen | `Alive` |
| Not seen for half TTL | `Stale` |
| Not seen for full TTL | `Expired` |

```cpp
registry.refresh_states(
    core::time::Timestamp::now(),
    core::time::Duration::from_seconds(8),
    core::time::Duration::from_seconds(15));

registry.prune_expired();
```

## Local development mode

Use `DiscoveryConfig::local()` or `DiscoveryOptions::local()` when running several nodes on the same machine:

```cpp
auto options = discovery::DiscoveryOptions::local("node-a", 9400, 7000);
```

Uses localhost addresses.

## LAN mode

Use `DiscoveryConfig::lan()` or `DiscoveryOptions::lan()` when discovering nodes on a local network:

```cpp
auto options = discovery::DiscoveryOptions::lan("node-a", 9400, 7000);
```

Uses broadcast by default.

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
cmake --build build

# or with preset
cmake --build --preset default
```

## Common mistakes

### Transport engine not started

`DiscoveryContext::is_valid()` expects the transport engine to be running.

Correct order:

```
1. Start TransportEngine
2. Create DiscoveryContext
3. Start DiscoveryEngine or DiscoveryService
```

### `announce_port` is zero

`announce_port` must be the transport port other nodes should use to connect:

```cpp
// 9400 -> UDP discovery port
// 7000 -> transport port announced to peers
auto options = discovery::DiscoveryOptions::local("node-a", 9400, 7000);
```

### Using expired peers

Expired peers should not be used for new transport connections. Use:

```cpp
registry.available_peers();
registry.transport_peers();
```

## Production notes

The current UDP backend is intentionally simple. Recommended next steps:

- async UDP backend
- multicast backend
- configurable network interfaces
- signed and encrypted announcements
- peer trust policy
- rate limiting
- persistent peer cache
- structured metrics and tracing
- edge node and P2P discovery integration

## Design rules

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

## Summary

`softadastra/discovery` answers one question:

> *Which peers are available nearby?*

Once a peer is found, discovery hands it to transport. Transport then connects and moves sync messages.
