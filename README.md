# Softadastra Discovery

> **Peer discovery for real-world networks.**

Softadastra Discovery is a lightweight, offline-first discovery layer designed to find peers reliably across unstable networks.

It allows nodes to:

- discover each other over LAN or local networks
- announce their presence
- detect peers in real time
- integrate directly with the transport layer

Built for environments where connectivity is unreliable, intermittent, or local-first.

## Why Discovery matters

In real-world systems:

- networks are unstable
- DNS is not always reliable
- central registries are not always available
- devices must find each other locally

Softadastra Discovery solves this by enabling:

> **Zero-configuration peer discovery over local networks.**

No central server. No coordination. Just working systems.

## Key Features

- 🔍 Automatic peer discovery (broadcast + probe)
- ⚡ Real-time detection of peers
- 🌍 Works without internet (LAN / local networks)
- 🔁 Designed for offline-first systems
- 🔌 Direct integration with `TransportEngine`
- 🧠 Clean separation of concerns (backend / engine / service)
- 🧩 Simple public API via `DiscoveryService`

## Installation

```bash
vix add @softadastra/discovery
vix install
```

## Quick Start

```cpp
#include <softadastra/discovery/discovery.hpp>

using namespace softadastra::discovery;

DiscoveryOptions options;
options.node_id        = "node-a";
options.bind_port      = 9400;
options.broadcast_port = 9400;
options.announce_port  = 9301;

DiscoveryService discovery(options, transport_engine);

discovery.onPeerFound([](const Peer& peer) {
    std::cout << "Peer found: "
              << peer.node_id
              << " at "
              << peer.host
              << ":"
              << peer.port
              << std::endl;
});

discovery.start();

for (;;) {
    discovery.poll_many(16);
}
```

## Concepts

### `DiscoveryService`

The main entry point of the module. Responsible for:

- starting discovery
- polling network messages
- emitting peer events
- exposing discovered peers

### `DiscoveryOptions`

Simple configuration structure:

```cpp
DiscoveryOptions options;
options.node_id        = "node-a";
options.bind_port      = 9400;
options.broadcast_port = 9400;
options.announce_port  = 9301;
```

### `Peer`

Represents a discovered node:

```cpp
Peer peer;
peer.node_id;  // unique identifier
peer.host;     // IP address
peer.port;     // port number
```

## How it works

1. Nodes broadcast announcements over UDP
2. Nodes send probes to discover peers
3. Incoming messages are decoded and processed
4. Peers are tracked in a local registry
5. New peers trigger events (`onPeerFound`)
6. Discovery integrates with the transport layer automatically

## Architecture

```
DiscoveryService      (public API)
        ↓
DiscoveryEngine       (orchestration)
        ↓
DiscoveryClient / DiscoveryServer
        ↓
IDiscoveryBackend     (UDP)
        ↓
Platform              (Linux / future OS)
```

## Examples

Build the examples:

```bash
vix build
```

| Example | Command |
|---------|---------|
| Minimal | `./build-ninja/examples/discovery_minimal` |
| Listener | `./build-ninja/examples/discovery_listener` |
| Announcer | `./build-ninja/examples/discovery_announcer` |
| Roundtrip demo (2 nodes) | `./build-ninja/examples/discovery_roundtrip_demo` |

## Designed for the real world

Softadastra Discovery is not built for ideal conditions.

It is built for:

- unstable networks
- intermittent connectivity
- local-first systems
- peer-to-peer environments

## Position in Softadastra

Softadastra Discovery is part of the core stack:

```
Discovery → Transport → Sync → Store → WAL
```

It is the entry point for:

- finding peers
- connecting nodes
- enabling synchronization

## Roadmap

- [ ] Multi-platform support (macOS, Windows)
- [ ] Multicast support
- [ ] Encrypted discovery channels
- [ ] NAT traversal strategies
- [ ] Pluggable backends (beyond UDP)

## License

MIT

## Softadastra

Softadastra is a foundational system for building reliable software in unreliable environments.

> *Write locally. Persist first. Sync later.*
