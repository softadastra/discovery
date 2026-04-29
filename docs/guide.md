# Discovery Guide

The Softadastra Discovery module finds nearby nodes and hands them to the transport layer.

The core rule is:

> Discovery finds peers. Transport connects peers.

## Why Discovery exists

Softadastra is built for local-first and offline-first systems.

In that model, nodes must be able to discover each other without depending on a central cloud server.

Discovery provides that first local peer discovery layer.

It allows a node to:

- announce itself
- probe the local network
- receive peer announcements
- track discovered peers
- expire stale peers
- convert discovered peers into transport peers
- hand peers to `softadastra/transport`

## Layering

Softadastra separates responsibilities clearly:

```text id="discovery-layering"
WAL        -> durable operation log
Store      -> local state
Sync       -> operation propagation
Transport  -> peer message delivery
Discovery  -> peer discovery

Discovery does not apply operations.

Discovery does not connect TCP peers directly.

Discovery only finds peers and gives their information to transport.

What Discovery does

softadastra/discovery provides:

discovery message types
discovery peer states
discovery engine status
discovery announcements
discovery messages
discovery envelopes
UDP datagrams
message encoding and decoding
discovered peer registry
UDP discovery backend
discovery client
discovery server
discovery engine
high-level discovery service
public peer object
What Discovery does not do

Discovery does not implement:

WAL persistence
store mutation
sync conflict resolution
sync queueing
TCP transport
sync message delivery after peer discovery
encryption
authentication
consensus

Those belong to other Softadastra modules.

Installation
vix add @softadastra/discovery
Main header

Use the public aggregator:

#include <softadastra/discovery/Discovery.hpp>

For full integration:

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>
#include <softadastra/discovery/Discovery.hpp>
Main concepts

The Discovery module is built around these concepts:

DiscoveryMessageType
DiscoveryPeerState
DiscoveryStatus
DiscoveryConfig
DiscoveryOptions
DiscoveryAnnouncement
DiscoveryMessage
DiscoveryEnvelope
Datagram
DiscoveryEncoder
DiscoveryDecoder
DiscoveredPeer
DiscoveryRegistry
IDiscoveryBackend
UdpDiscoveryBackend
DiscoveryClient
DiscoveryServer
DiscoveryContext
DiscoveryEngine
DiscoveryService
Peer
Message types

Discovery supports three message types:

discovery::types::DiscoveryMessageType::Announce
discovery::types::DiscoveryMessageType::Probe
discovery::types::DiscoveryMessageType::Reply
Announce

An Announce message tells nearby nodes:

I exist.
Here is my node id.
Here is the host and port you can use to reach me.
Probe

A Probe message asks nearby nodes:

Who is available?
Reply

A Reply message answers a probe with local peer information.

Helpers:

discovery::types::to_string(type);
discovery::types::is_valid(type);
discovery::types::is_announcement(type);
discovery::types::is_probe_flow(type);
Peer states

A discovered peer can be in one of these states:

discovery::types::DiscoveryPeerState::Discovered
discovery::types::DiscoveryPeerState::Alive
discovery::types::DiscoveryPeerState::Stale
discovery::types::DiscoveryPeerState::Expired
Discovered

The peer has been seen at least once.

Alive

The peer has been seen recently and can be used.

Stale

The peer has not been seen recently, but it has not fully expired.

Expired

The peer has exceeded its TTL and should be ignored or removed.

Helpers:

discovery::types::to_string(state);
discovery::types::is_valid(state);
discovery::types::is_available(state);
discovery::types::is_unavailable(state);
discovery::types::is_expired(state);
Discovery status

The engine status can be:

discovery::types::DiscoveryStatus::Stopped
discovery::types::DiscoveryStatus::Starting
discovery::types::DiscoveryStatus::Running
discovery::types::DiscoveryStatus::Stopping
discovery::types::DiscoveryStatus::Failed

Helpers:

discovery::types::to_string(status);
discovery::types::is_valid(status);
discovery::types::is_running(status);
discovery::types::is_transitioning(status);
discovery::types::is_terminal(status);
DiscoveryConfig

DiscoveryConfig is the low-level runtime configuration.

Create a local development config:

auto config =
    discovery::core::DiscoveryConfig::local(
        "node-a",
        9400,
        7000);

Meaning:

node-a -> local node id
9400   -> UDP discovery bind port
7000   -> transport port announced to peers

Create a LAN broadcast config:

auto config =
    discovery::core::DiscoveryConfig::lan(
        "node-a",
        9400,
        7000);

Customize timing:

config.announce_interval =
    core::time::Duration::from_seconds(3);

config.peer_ttl =
    core::time::Duration::from_seconds(15);

Validate:

if (!config.is_valid())
{
  return 1;
}
DiscoveryOptions

DiscoveryOptions is the user-facing configuration used by DiscoveryService.

Create local options:

auto options =
    discovery::DiscoveryOptions::local(
        "node-a",
        9400,
        7000);

Create LAN options:

auto options =
    discovery::DiscoveryOptions::lan(
        "node-a",
        9400,
        7000);

Convert to core config:

auto config =
    options.to_config();

Validate:

if (!options.is_valid())
{
  return 1;
}

Backward-compatible alias:

options.valid();
DiscoveryAnnouncement

A DiscoveryAnnouncement is the payload that advertises a node.

It contains:

node_id
host
port
timestamp

Create an announcement:

discovery::core::DiscoveryAnnouncement announcement{
    "node-a",
    "127.0.0.1",
    7000};

Create a local announcement:

auto announcement =
    discovery::core::DiscoveryAnnouncement::local(
        "node-a",
        7000);

Validate:

if (!announcement.is_valid())
{
  return 1;
}

Refresh timestamp:

announcement.touch();
DiscoveryMessage

DiscoveryMessage is the logical discovery message exchanged between nodes.

Create an announce message:

auto message =
    discovery::core::DiscoveryMessage::announce(
        "node-a",
        payload);

Create a probe message:

auto probe =
    discovery::core::DiscoveryMessage::probe("node-a");

Create a reply message:

auto reply =
    discovery::core::DiscoveryMessage::reply(
        "node-a",
        payload);

Add routing metadata:

message.to_node_id = "node-b";
message.correlation_id = "announce-1";

Validate:

if (!message.is_valid())
{
  return 1;
}
DiscoveryEnvelope

DiscoveryEnvelope wraps a discovery message with network metadata.

It tracks:

message
from_host
from_port
to_host
to_port
timestamp
retry_count
last_attempt_at

Create an outbound envelope:

discovery::core::DiscoveryEnvelope envelope{
    message,
    "127.0.0.1",
    9400};

Validate before sending:

if (envelope.is_valid() && envelope.has_destination())
{
  envelope.mark_attempt();
}
Datagram

Datagram is the raw UDP payload container used by the backend.

discovery::utils::Datagram datagram{
    "127.0.0.1",
    9400,
    {1, 2, 3}};

Validate:

if (datagram.is_valid())
{
  auto size = datagram.payload_size();
}
Encoding

Encode a discovery message:

auto encoded =
    discovery::encoding::DiscoveryEncoder::encode_message(message);

Wrap a discovery message into a datagram:

auto datagram =
    discovery::encoding::DiscoveryEncoder::make_datagram(
        message,
        "127.0.0.1",
        9400);
Decoding

Decode a message from bytes:

auto decoded =
    discovery::encoding::DiscoveryDecoder::decode_message(encoded);

Decode a message from a datagram:

auto decoded =
    discovery::encoding::DiscoveryDecoder::decode_datagram(datagram);
DiscoveredPeer

DiscoveredPeer is the internal runtime representation of one discovered peer.

discovery::peer::DiscoveredPeer peer{announcement};

peer.mark_alive();

if (peer.available())
{
  auto transport_peer =
      peer.to_peer_info();
}

State helpers:

peer.discovered();
peer.alive();
peer.stale();
peer.expired();
peer.available();

Runtime updates:

peer.mark_discovered();
peer.mark_alive();
peer.mark_stale();
peer.mark_expired();
peer.mark_error();
peer.touch();

Update from a new announcement:

peer.update(new_announcement);

Convert to transport:

auto transport_peer =
    peer.to_peer_info();
DiscoveryRegistry

DiscoveryRegistry stores discovered peers in memory.

discovery::peer::DiscoveryRegistry registry;

registry.upsert_announcement(announcement);

Find without copying:

auto *peer =
    registry.find("node-b");

if (peer != nullptr && peer->alive())
{
  // peer is alive
}

Inspect peers:

registry.size();
registry.available_peers();
registry.alive_peers();
registry.stale_peers();
registry.expired_peers();

Convert discovered peers to transport peers:

auto transport_peers =
    registry.transport_peers();

Refresh stale and expired states:

registry.refresh_states(
    core::time::Timestamp::now(),
    core::time::Duration::from_seconds(8),
    core::time::Duration::from_seconds(15));

Prune expired peers:

registry.prune_expired();
Backend interface

IDiscoveryBackend defines the low-level discovery backend contract.

A backend must implement:

start()
stop()
is_running()
send(envelope)
poll()

Skeleton:

class MyDiscoveryBackend
    : public discovery::backend::IDiscoveryBackend
{
public:
  bool start() override
  {
    return true;
  }

  void stop() override
  {
  }

  bool is_running() const noexcept override
  {
    return true;
  }

  bool send(const discovery::core::DiscoveryEnvelope &envelope) override
  {
    return envelope.is_valid();
  }

  std::optional<discovery::core::DiscoveryEnvelope> poll() override
  {
    return std::nullopt;
  }
};
UDP backend

UdpDiscoveryBackend is the default Linux UDP backend.

auto config =
    discovery::core::DiscoveryConfig::local(
        "node-a",
        9400,
        7000);

discovery::backend::UdpDiscoveryBackend backend{config};

if (!backend.start())
{
  return 1;
}

backend.stop();

The current UDP backend is simple and blocking.

DiscoveryClient

DiscoveryClient is a thin outbound wrapper around a discovery backend.

discovery::client::DiscoveryClient client{backend};

client.send_probe(
    "127.0.0.1",
    9400,
    "node-a");

Send an announcement:

client.send_announce(
    config.broadcast_host,
    config.broadcast_port,
    config.node_id,
    payload);

Send a reply:

client.send_reply(
    "127.0.0.1",
    9400,
    "node-a",
    payload);
DiscoveryServer

DiscoveryServer is a thin inbound wrapper around a discovery backend.

discovery::server::DiscoveryServer server{backend};

server.start();

auto inbound =
    server.poll();

if (inbound.has_value())
{
  auto message = inbound->message;
}

server.stop();
DiscoveryContext

DiscoveryContext connects discovery to transport.

discovery::core::DiscoveryContext context{
    discovery_config,
    transport_engine};

if (!context.is_valid())
{
  return 1;
}

Access transport safely:

auto transport =
    context.transport_checked();

if (transport.is_ok())
{
  auto *engine = transport.value();
}
DiscoveryEngine

DiscoveryEngine is the high-level discovery facade.

It coordinates:

backend lifecycle
announce messages
probe messages
inbound polling
peer registry updates
transport integration

Create an engine:

discovery::core::DiscoveryConfig discovery_config =
    discovery::core::DiscoveryConfig::local(
        "node-a",
        9400,
        7000);

discovery::core::DiscoveryContext discovery_context{
    discovery_config,
    transport_engine};

discovery::backend::UdpDiscoveryBackend discovery_backend{
    discovery_config};

discovery::engine::DiscoveryEngine discovery_engine{
    discovery_context,
    discovery_backend};

Start it:

if (!discovery_engine.start())
{
  return 1;
}

Send an announcement:

discovery_engine.announce_now();

Send a probe:

discovery_engine.probe_now();

Poll once:

discovery_engine.poll_once();

Poll many:

auto processed =
    discovery_engine.poll_many(16);

Read discovered transport peers:

auto peers =
    discovery_engine.available_transport_peers();

Stop:

discovery_engine.stop();
DiscoveryService

DiscoveryService is the simple user-facing API.

It owns:

DiscoveryOptions
DiscoveryConfig
DiscoveryContext
UdpDiscoveryBackend
DiscoveryEngine

It does not own TransportEngine.

The transport engine must outlive the discovery service.

auto options =
    discovery::DiscoveryOptions::local(
        "node-a",
        9400,
        7000);

discovery::DiscoveryService service{
    options,
    transport_engine};

Start service:

if (!service.start())
{
  return 1;
}

Install callback:

service.on_peer_found(
    [](const discovery::Peer &peer)
    {
      // peer discovered
    });

Announce and probe:

service.announce_now();
service.probe_now();

Poll:

service.poll();
service.poll_many(16);

Get public peers:

auto peers =
    service.peers();

Stop:

service.stop();
Public Peer

discovery::Peer is the simple public representation of a discovered peer.

discovery::Peer peer{
    "node-b",
    "127.0.0.1",
    7001};

Validate:

if (!peer.is_valid())
{
  return 1;
}

Convert to transport peer:

auto transport_peer =
    peer.to_transport_peer();

Create from internal discovered peer:

auto public_peer =
    discovery::Peer::from_discovered_peer(discovered_peer);
Full integration setup

Discovery normally runs after transport.

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

  auto sync_config =
      sync::core::SyncConfig::durable("node-a");

  sync::core::SyncContext sync_context{
      store,
      sync_config};

  sync::engine::SyncEngine sync_engine{
      sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7000);

  transport::core::TransportContext transport_context{
      transport_config,
      sync_engine};

  transport::backend::TcpTransportBackend transport_backend{
      transport_config};

  transport::engine::TransportEngine transport_engine{
      transport_context,
      transport_backend};

  if (!transport_engine.start())
  {
    return 1;
  }

  auto discovery_options =
      discovery::DiscoveryOptions::local(
          "node-a",
          9400,
          7000);

  discovery::DiscoveryService discovery_service{
      discovery_options,
      transport_engine};

  discovery_service.on_peer_found(
      [](const discovery::Peer &peer)
      {
        std::cout << "peer found: "
                  << peer.node_id
                  << " "
                  << peer.host
                  << ":"
                  << peer.port
                  << "\n";
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
Discovery flow
1. TransportEngine starts
2. DiscoveryEngine or DiscoveryService starts
3. Node sends Announce over UDP
4. Other nodes receive the announcement
5. DiscoveryRegistry stores or refreshes the peer
6. DiscoveredPeer becomes Alive
7. DiscoveryEngine converts it to transport::core::PeerInfo
8. TransportEngine connects to the peer
9. Sync can send messages through Transport
Probe flow
1. Node sends Probe
2. Nearby nodes receive Probe
3. Each node replies with Reply
4. Reply contains an encoded DiscoveryAnnouncement
5. Receiver stores the peer
6. Receiver integrates peer with Transport
Expiration flow

Peers are refreshed every time a valid announcement or reply is received.

Recently seen peer      -> Alive
Not seen for half TTL   -> Stale
Not seen for full TTL   -> Expired

Refresh states manually:

registry.refresh_states(
    core::time::Timestamp::now(),
    core::time::Duration::from_seconds(8),
    core::time::Duration::from_seconds(15));

Prune expired peers:

registry.prune_expired();
Local development mode

Use DiscoveryConfig::local() or DiscoveryOptions::local() when running several nodes on the same machine.

auto options =
    discovery::DiscoveryOptions::local(
        "node-a",
        9400,
        7000);

This uses localhost addresses.

LAN mode

Use DiscoveryConfig::lan() or DiscoveryOptions::lan() when discovering nodes on a local network.

auto options =
    discovery::DiscoveryOptions::lan(
        "node-a",
        9400,
        7000);

This uses broadcast by default.

Examples

Available examples:

discovery_minimal.cpp
discovery_announcer.cpp
discovery_listener.cpp
discovery_roundtrip_demo.cpp
discovery_codec.cpp
discovery_registry.cpp
discovery_service.cpp
discovery_engine_setup.cpp

Build examples:

cmake --build build

or with preset:

cmake --build --preset default
Common mistakes
Transport engine not started

DiscoveryContext::is_valid() expects the transport engine to be running.

Correct order:

1. Start TransportEngine
2. Create DiscoveryContext
3. Start DiscoveryEngine or DiscoveryService
announce_port is zero

announce_port must be the transport port other nodes should use to connect.

auto options =
    discovery::DiscoveryOptions::local(
        "node-a",
        9400,
        7000);

Here:

9400 -> UDP discovery port
7000 -> transport port announced to peers
Using expired peers

Expired peers should not be used for new transport connections.

Use:

registry.available_peers();
registry.transport_peers();

instead of manually reading all entries.

Production notes

The current UDP backend is intentionally simple.

For production-grade discovery, the next steps are:

async UDP backend
multicast backend
configurable network interfaces
signed announcements
encrypted discovery payloads
peer trust policy
rate limiting
persistent peer cache
structured metrics
discovery tracing
edge node discovery
P2P discovery integration
Design rules

Keep these rules stable:

Discovery finds peers.
Transport connects peers.
Sync owns operation meaning.
Store owns state.
WAL owns durability.
Discovery must not mutate store.
Discovery must not apply sync operations.
Discovery must not resolve conflicts.
Discovery must keep peer state observable.
Backends must not contain registry logic.
Backends must not contain transport integration logic.
Summary

softadastra/discovery is the peer discovery layer.

It answers one question:

Which peers are available nearby?

Once a peer is found, discovery hands it to transport.

Transport then connects and moves sync messages.
