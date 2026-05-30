# softadastra/discovery

> Local peer discovery primitives for reliable Softadastra products.

`softadastra/discovery` provides the peer discovery layer of the Softadastra C++ stack.

Softadastra builds reliability-first products for local-first, offline-first, and distributed applications. This module helps nearby nodes find each other without depending on a central cloud service.

## Purpose

`softadastra/discovery` exists to find available peers and hand them to the transport layer.

It is used by higher-level SDKs, product APIs, LAN-first systems, and distributed Softadastra infrastructure.

The core rule is simple:

> Discovery finds peers. Transport connects peers.

It is designed to be:

- Local-first
- Lightweight
- Peer-aware
- Transport-friendly
- Observable
- Product-ready

## What it provides

This module provides discovery primitives such as:

- Discovery messages
- Peer announcements
- Discovery envelopes
- Datagram encoding and decoding
- Discovered peer registry
- UDP discovery backend
- Discovery client and server wrappers
- Discovery engine
- High-level discovery service

## What it does not do

`softadastra/discovery` does not contain:

- Durable persistence
- Store mutation
- Sync queueing
- Conflict resolution
- TCP connection handling
- Message transport after discovery
- Encryption
- Distributed consensus
- Product-specific logic

It finds peers and exposes their connection information. The transport layer decides how to connect and send messages.

## Core model

```txt
Local node
    |
Announce / Probe
    |
Discovered peers
    |
Peer registry
    |
Transport connection
```

Discovery keeps peer discovery separate from transport, sync, storage, and durability.

## Where it fits

```txt
Softadastra products
        |
SDKs and product APIs
        |
softadastra/discovery
        |
softadastra/transport
        |
softadastra/sync
        |
softadastra/store
        |
softadastra/wal
        |
softadastra/core
```

`softadastra/discovery` works with `softadastra/transport` to turn nearby nodes into connectable peers.

## Installation

```bash
vix add @softadastra/discovery
```

## Usage

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

## Example

```cpp
#include <softadastra/discovery/Discovery.hpp>

#include <iostream>

int main()
{
    auto options = softadastra::discovery::DiscoveryOptions::local(
        "node-a",
        9400,
        7000
    );

    if (!options.is_valid())
    {
        std::cout << "invalid discovery options\n";
        return 1;
    }

    auto config = options.to_config();

    auto announcement =
        softadastra::discovery::core::DiscoveryAnnouncement::local(
            config.node_id,
            config.transport_port
        );

    if (!announcement.is_valid())
    {
        std::cout << "invalid announcement\n";
        return 1;
    }

    std::cout << "Discovery node: " << announcement.node_id << "\n";
    std::cout << "Transport port: " << announcement.port << "\n";

    return 0;
}
```

## Requirements

- C++20
- `softadastra/core`
- `softadastra/transport`
- Platform UDP APIs when UDP discovery backends are enabled

## Documentation

For the full documentation, visit [docs.softadastra.com](https://docs.softadastra.com).

## License

Licensed under the Apache License, Version 2.0.
